#include "apparatus.h"

#include "asset_manager.h"

#include "pfd/portable-file-dialogs.h"

struct Asset_Directory {
    Asset_Directory(Asset_Directory* parent, str_ptr_t path) 
        : parent_directory(parent) {
        Path::name_with_extension(path, name);
        strcpy(real_path, path);
    }

    Asset_Directory* parent_directory;

    path_str_t name;
    path_str_t real_path;

    Hash_Set<asset_id_t> assets;

    Dynamic_Array<Asset_Directory*> sub_directories;

    void traverse(const std::function<void(Asset_Directory&)> fn) {
        for (auto& sub : sub_directories) {
            sub->traverse(fn);
        }
        fn(*this);
    }

    void clear() {
        assets.clear();
        for (auto sub_dir : sub_directories) {
            sub_dir->clear();
            delete sub_dir;
        }
        sub_directories.clear();
    }
};

Gui_Window g_asset_manager = { true, "Asset Manager" };
Gui_Window g_asset_inspector = { false, "Asset Inspector" };

Gui_Popup g_manager_menu_popup;

path_str_t g_current_dir;
path_str_t g_assets_dir;

Dynamic_Array<Asset> assets;
Dynamic_Array<byte> runtime_data;

// Sorted by usage frequency
Dynamic_Array<asset_id_t> order_to_unload_assets;

u32 number_of_assets_in_use = 0;

Graphics_Context* g_graphics;

Asset_Directory* g_root_directory = NULL;
Asset_Directory* g_selected_dir = NULL;

constexpr str_ptr_t disk_file_name = "assetlist";

Hash_Set<u32> taken_uids;

u32 get_index(asset_id_t id) {
    u32 idx = 0;
    memcpy(&idx, &id, 4);
    return idx;
}

u32 get_uid(asset_id_t id) {
    u32 uid = 0;
    memcpy(&uid, (byte*)&id + 4, 4);
    return uid;
}

void set_index(asset_id_t* id, u32 index) {
    byte* ptr = (byte*)id;
    memcpy(ptr, &index, sizeof(u32));
}

void set_uid(asset_id_t* id, u32 uid) {
    byte* ptr = (byte*)id;
    memcpy(ptr + 4, &uid, sizeof(u32));
}

asset_id_t make_id(u32 index, u32 uid) {
    asset_id_t id = 0;
    memcpy(&id, &index, 4);
    memcpy(((byte*)&id) + 4, &uid, 4);
    return id;
}

u32 generate_uid() {
    
    u32 uid = rand() % UINT_MAX;
    while (taken_uids.find(uid) != taken_uids.end() && uid == (u32)-1) {
        uid = rand() % UINT_MAX;
    }
    taken_uids.emplace(uid);
    return uid;
}

bool is_uid_available(u32 uid) {
    return taken_uids.find(uid) == taken_uids.end() && uid != (u32)-1;
}

data_index_t load_texture(Asset* asset) {
    Texture_Data tex;
    
    tex.graphics_id = g_graphics->make_texture(G_BUFFER_USAGE_STATIC_WRITE);
    auto* img = load_image_from_file(asset->path, &tex.size.x, &tex.size.y, &tex.channels, 4);

    if (!img) {
        log_error("Failed loading texture from: \n{}\nReason: ", get_failure_reason());
        return NULL_DATA_INDEX;
    }

    g_graphics->set_texture_filtering(tex.graphics_id, G_MIN_FILTER_NEAREST, G_MAG_FILTER_NEAREST);
    g_graphics->set_texture_wrapping(tex.graphics_id, G_WRAP_CLAMP_TO_BORDER);

    static std::thread::id this_thread = std::this_thread::get_id();
    ap_assert(this_thread == std::this_thread::get_id(), "I sayeth, Nay Thee! ({}, {})", this_thread, std::this_thread::get_id());

    size_t img_size = tex.size.width * tex.size.height * tex.channels;

    data_index_t data_index = runtime_data.size();
    runtime_data.resize(runtime_data.size() + sizeof(Texture_Data) + img_size);
    tex.data = &runtime_data[data_index + sizeof(Texture_Data)];

    memcpy(&runtime_data[data_index], &tex, sizeof(Texture_Data));
    memcpy(&runtime_data[data_index + sizeof(Texture_Data)], img, img_size);

    g_graphics->set_texture_data(tex.graphics_id, (byte*)tex.data, tex.size, G_TEXTURE_FORMAT_RGBA);

    free_image(img);

    return data_index;
}

bool unload_texture(Asset* asset) {
    asset->is_garbage = true;

    auto& tex = *(Texture_Data*)asset->get_runtime_data();
    
    g_graphics->destroy_texture(tex.graphics_id);

    return true;
}

asset_id_t register_asset(str_ptr_t path, asset_type_t asset_type, u32 uid = (u32)-1) {
    
    Asset a;
    a.is_garbage = false;
    strcpy(a.path, path);
    Path::extension_of(path, a.extension);
    Path::name_with_extension(path, a.file_name);
    Path::name_without_extension(path, a.name);

    a.asset_type = asset_type;
    a.data_stream = &runtime_data;

    u32 index = (u32)-1;

    for (int i = 0; i < assets.size(); i++) {
        if (assets[i].is_garbage) {
            assets[i] = std::move(a);
            index = i;
        }
    }

    if (index == (u32)-1) {
        assets.push_back(std::move(a));
        index = (u32)assets.size() - 1;
    }

    if (uid == (u32)-1) {
        uid = generate_uid();
    } else {
        ap_assert(is_uid_available(uid), "uid taken");
    }

    asset_id_t id = make_id(index, uid);

    log_info("Registered asset:\nPath: {}\nName: {}, {}\nExt:  {}\nid:   {} (idx: {}, uid: {})",
              a.path, a.name, a.file_name, a.extension, id, index, uid);

    assets[index].id = id;
    return id;
}

asset_id_t register_by_extension(str_ptr_t path, u32 uid = (u32)-1) {
    str16_t ext = "";
    Path::extension_of(path, ext);
    if (strcmp(ext, "png") == 0) {
        auto id = register_asset(path, ASSET_TYPE_TEXTURE, uid);
        
        return id;
    } else {
        log_error("Cannot load files with extension '{}'", ext);
        return NULL_ASSET_ID;
    }
}

data_index_t load_to_memory(asset_id_t id) {
    u32 index = get_index(id);
    ap_assert(index < assets.size());
    Asset* asset = &assets[index];
    
    data_index_t data_index = load_texture(asset);
    
    if (data_index == NULL_DATA_INDEX) {
        return NULL_DATA_INDEX;
    }

    asset->in_memory = true;

    return data_index;
}

bool unload_from_memory(asset_id_t id) {
    u32 index = get_index(id);
    ap_assert(index < assets.size());
    Asset* asset = &assets[index];
    ap_assert(!asset->in_use, "Cannot unload an asset that's in use");

    if (!unload_texture(asset)) return false;

    asset->in_memory = false;

    return true;
}

void delete_asset(asset_id_t id) {
    ap_assert(number_of_assets_in_use == 0, "Cannot delete an asset when there are assets in use.");
    u32 index = get_index(id);
    ap_assert(index < assets.size());
    Asset& asset = assets[index];

    if (asset.in_memory) {
        unload_from_memory(id);
    }
}

Asset* begin_use(asset_id_t id) {
    u32 index = get_index(id);
    ap_assert(index < assets.size());
    number_of_assets_in_use++;

    Asset* asset = &assets[index];
    asset->in_use = true;
    asset->usage_points++;

    if (!asset->in_memory) {
        asset->data_index = load_to_memory(id);
    }

    return asset;
}

void end_use(asset_id_t id) {
    u32 index = get_index(id);
    ap_assert(index < (u32)assets.size());
    Asset* asset = &assets[index];
    ap_assert(asset->in_use, "Cannot stop using an asset that's not in use");
    asset->in_use = false;

    number_of_assets_in_use--;
}

void do_asset_manager_menu_gui(Asset_Directory* dir) {

    if (ImGui::BeginMenu("Create")) {

        static name_str_t create_name = "";

        ImGui::RInputText("Name", create_name, sizeof(create_name));

        if (ImGui::MenuItem("Directory")) {
            path_str_t new_path = "";
            sprintf(new_path, "%s/%s", dir->real_path, create_name);
            if (Path::create_directory(new_path)) {
                Asset_Directory* new_dir = new Asset_Directory(dir, new_path);
                dir->sub_directories.push_back(new_dir);
            } else {
                log_error("Failed creating directory '{}'", new_path);
            }
        }

        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Add")) {
        open_file_browser(File_Browser_Mode::file, [dir](str_ptr_t result) {
            str_ptr_t result_path = result;
            if (Path::exists(result_path)) {
                path_str_t new_path = "";
                path_str_t file_name = "";
                Path::name_with_extension(result_path, file_name);
                sprintf(new_path, "%s/%s", dir->real_path, file_name);
                Path::copy(result_path, new_path);

                auto id = register_by_extension(new_path);
                if (id != NULL_ASSET_ID) {
                    dir->assets.emplace(id);
                }
            }
        });
    }
}

void clear_assets() {
    for (auto& asset : assets) {
        ap_assert(!asset.in_use, "Asset is left in use when asset system is loaded");
        if (asset.in_memory) unload_from_memory(asset.id);
    }
    assets.clear();
    if (g_root_directory) g_root_directory->clear();
    runtime_data.clear();
    taken_uids.clear();
}

extern "C" {
    _export void __cdecl on_load(Graphics_Context* graphics) {
        (void)graphics;
        srand((u32)time(NULL));

        path_str_t folder_icon_path = "";
        path_str_t file_icon_path = "";

        graphics->set_clear_color(mz::color(.35f, .1f, .65f, 1.f));

        register_gui_window(&g_asset_manager);
        register_gui_window(&g_asset_inspector);

        strcpy(g_current_dir, get_executable_directory());

        g_manager_menu_popup.is_modal = false;
        strcpy(g_manager_menu_popup.str_id, "Asset Manager Menu");
        g_manager_menu_popup.fn = []() {
            do_asset_manager_menu_gui(g_selected_dir);
        };

        register_gui_popup(&g_manager_menu_popup);

        g_graphics = graphics;
    }

    _export void __cdecl save_to_disk(str_ptr_t dir) {
        path_str_t file_path = "";
        sprintf(file_path, "%s/%s", dir, disk_file_name);

        std::ofstream ostream;
        ostream.open(file_path);

        for (u32 i = 0; i < assets.size(); i++) {
            if (assets[i].is_garbage) continue;

            path_str_t rel_path = "";
            Path::to_relative(assets[i].path, dir, rel_path);
            ostream << assets[i].id << " " << rel_path << "\n";
        }

        ostream.close();
    }

    _export void __cdecl load_from_disk(str_ptr_t dir) {
        clear_assets();

        sprintf(g_assets_dir, "%s/assets", dir);

        if (g_root_directory) delete g_root_directory;
        g_root_directory = new Asset_Directory(NULL, g_assets_dir);
        g_selected_dir = g_root_directory;

        path_str_t file_path = "";
        sprintf(file_path, "%s/%s", dir, disk_file_name);

        sprintf(g_assets_dir, "%s/assets", dir);
        if (!Path::exists(g_assets_dir)) Path::create_directory(g_assets_dir);

        if (!Path::exists(file_path)) {
            log_warn("No assets file found at\n{}", file_path);
            return;
        }


        File_Info info;
        Path::get_info(file_path, &info);

        char* data = (char*)malloc(info.size);

        Path::read_all_bytes(file_path, (byte*)data, info.size);

        Path::iterate_directories(g_assets_dir, [](str_ptr_t entry) {
            if (Path::is_directory(entry)) {
                path_str_t entry_parent_dir = "";
                Path::directory_of(entry, entry_parent_dir);
                g_root_directory->traverse([&](Asset_Directory& dir) {
                    if (Path::equals(dir.real_path, entry_parent_dir)) {
                        dir.sub_directories.push_back(new Asset_Directory(&dir, entry));
                    }
                });
            }
        }, true);

        str_t<32> id_str = "";
        path_str_t rel_path = "";
        u8 stage = 0;
        
        path_str_t str = "";
        for (int i = 0; i < info.size; i++) {
            
            if (data[i] == ' ' && stage == 0) {
                stage = 1;
                strcpy(id_str, str);
                memset(str, 0, sizeof(str));
            } else if (data[i] == '\n' || data[i] == '\r' || data[i] == '\r\n' && stage == 1) {
                stage = 0;
                strcpy(rel_path, str);
                memset(str, 0, sizeof(str));

                path_str_t asset_path = "";
                sprintf(asset_path, "%s/%s", dir, rel_path);
                if (Path::can_open(asset_path)) {
                    asset_id_t aid = register_by_extension(asset_path, get_uid(atoll(id_str)));
                    path_str_t asset_dir = "";
                    Path::directory_of(asset_path, asset_dir);
                    g_root_directory->traverse([&](Asset_Directory& dir) {
                        if (Path::equals(dir.real_path, asset_dir)) {
                            dir.assets.emplace(aid);
                        }
                    });
                }
            } else {
                char c[2]; c[1] = '\0'; c[0] = data[i];
                strcat(str, c);
            }
        }

        free(data);
    }

    _export void __cdecl on_unload(Graphics_Context* graphics) {
        (void)graphics;

        clear_assets();

        unregister_gui_window(&g_asset_manager);
            unregister_gui_window(&g_asset_inspector);

        unregister_gui_popup(&g_manager_menu_popup);
    }

    _export void __cdecl on_update(float delta) {
        (void)delta;


    }

    _export void __cdecl on_render(Graphics_Context* graphics) {
        auto windows = graphics->get_windows_context();
        auto wnd = windows->main_window_handle;

        if (windows->should_close(wnd)) {
            quit();
        }
    }

    void do_dir_gui(Asset_Directory& dir, int depth = 0) {

        ImGuiTreeNodeFlags flags = 0;
        flags |= ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick;
        flags |= ImGuiTreeNodeFlags_SpanFullWidth;
        
        if (strcmp(dir.real_path, g_selected_dir->real_path) == 0) 
            flags |= ImGuiTreeNodeFlags_Selected;
        path_str_t node_id = "";
        sprintf(node_id, "##%i", depth);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        bool node_open = ImGui::TreeNodeEx(node_id, flags);
        ImGui::SameLine();
        ImGui::Icon(ICON_TYPE_FOLDER, { 16, 16 });
        ImVec2 vline_start = { ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y };
        vline_start.x -= 8;
        ImVec2 vline_end = vline_start;
        ImGui::SameLine();
        ImGui::Text(dir.name);

        if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) {
            g_selected_dir = &dir;
        }

        if (ImGui::BeginDragDropTarget()) {
            auto* p = ImGui::AcceptDragDropPayload("asset");
            if (p) {
                auto payload = (Gui_Payload*)p->Data;
                auto asset_id = (asset_id_t)(uintptr_t)payload->value;
                auto* prev_dir = (Asset_Directory*)payload->home;

                prev_dir->assets.erase(asset_id);
                dir.assets.emplace(asset_id);

                Asset& asset = assets[get_index(asset_id)];
                path_str_t prev_path = "";
                strcpy(prev_path, asset.path);
                sprintf(asset.path, "%s/%s", dir.real_path, asset.file_name);

                Path::copy(prev_path, asset.path);
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked(1)) {
            g_manager_menu_popup.should_open = true;
        }

        if (node_open) {	
            for (auto sub : dir.sub_directories) {
                do_dir_gui(*sub, ++depth);
            }
            for (auto asset_id : dir.assets) {
                auto& asset = assets[get_index(asset_id)];

                if (asset.asset_type == ASSET_TYPE_TEXTURE) {
                    ImGui::Icon(ICON_TYPE_TEXTURE, { 16, 16 });
                } else {
                    ImGui::Icon(ICON_TYPE_FILE, { 16, 16 });
                }
                vline_end = ImGui::GetItemRectMin();
                vline_end.y += 8;
                f32 x = vline_start.x > vline_end.x ? vline_end.x : vline_start.x;
                draw_list->AddLine(vline_end, { x, vline_end.y }, ImGui::GetColorU32(ImGuiCol_Text));
                ImGui::SameLine();
                if (ImGui::Selectable(asset.file_name)) {
                    
                }
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
                    Gui_Payload payload;
                    payload.home = &dir;
                    payload.value = (void*)(uintptr_t)asset_id;
                    ImGui::SetDragDropPayload("asset", &payload, sizeof(Gui_Payload));
                    ImGui::Text(asset.file_name);
                    ImGui::EndDragDropSource();
                }
            }
            f32 x = vline_start.x > vline_end.x ? vline_end.x : vline_start.x;
            vline_start.x = vline_end.x = x;
            draw_list->AddLine(vline_start, vline_end, ImGui::GetColorU32(ImGuiCol_Text));
            ImGui::TreePop();
        }
        depth++;
    }

    _export void __cdecl on_gui(Graphics_Context* graphics) {
        (void)graphics;

        ImGui::DoGuiWindow(&g_asset_manager, [&]() {

            ImGui::BeginMenuBar();

            do_asset_manager_menu_gui(g_selected_dir ? g_selected_dir : g_root_directory);

            ImGui::EndMenuBar();

            do_dir_gui(*g_root_directory);

        }, ImGuiWindowFlags_MenuBar);

        ImGui::DoGuiWindow(&g_asset_inspector, [&]() {

        });
    }

    _export void* __cdecl _request(void* ud) {
        Asset_Request* req = (Asset_Request*)ud;
        if (req->request_id == ASSET_REQUEST_REGISTER_ASSET) {
            auto* request = (Asset_Request_Register_Asset*)req;
            
            if (request->asset_type == ASSET_TYPE_TEXTURE) {
                return (void*)(uintptr_t)register_asset(request->path, request->asset_type);
            } else {
                log_error("Invalid asset type in register asset request");
                return NULL;
            }
        } else if (req->request_id == ASSET_REQUEST_BEGIN_USE_ASSET) {
            auto* request = (Asset_Request_Begin_Use_asset*)req;

            return begin_use(request->asset_id);
        } else if (req->request_id == ASSET_REQUEST_END_USE_ASSET) {
            auto* request = (Asset_Request_End_Use_Asset*)req;

            end_use(request->asset_id);
            return NULL;
        } else if (req->request_id == ASSET_REQUEST_CHECK_IF_VALID) {
            auto* request = (Asset_Request_Check_If_Valid*)req;
            u32 index = get_index(request->asset_id);
            return (void*)(uintptr_t)(index < assets.size() && !assets[index].is_garbage);
        } else if (req->request_id == ASSET_REQUEST_VIEW) {
            auto* request = (Asset_Request_View*)req;
            u32 index = get_index(request->asset_id);
            if (index < assets.size() && !assets[index].is_garbage)
                return &assets[index];
            else
                return NULL;
        }

        log_error("Invalid asset request");
        return NULL;
    }
}