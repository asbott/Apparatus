#include <random>

#include "apparatus.h"

#include "asset_manager.h"

void delete_asset(asset_id_t id);

struct Asset_Directory {
    Asset_Directory(Asset_Directory* parent, str_ptr_t path) 
        : parent_directory(parent) {
        Path::name_with_extension(path, name);
        Path::to_canonical(path, real_path);
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

    void delete_sub_dir(str_ptr_t real_path) {
        Asset_Directory* sub_dir = NULL;
        int idx = 0;
        for (int i = 0; i < (int)sub_directories.size(); i++) {
            if (Path::equals(sub_directories[i]->real_path, real_path) == 0) {
                sub_dir = sub_directories[i];
                idx = i;
                break;
            }
        }

        if (!sub_dir) return;

        for (auto aid : sub_dir->assets) {
            delete_asset(aid);
        }

        sub_dir->clear();

        delete sub_dir;

        sub_directories.erase(sub_directories.begin() + idx);
    }
};

struct Asset_Loader {
    Asset_Loader(const Asset_Loader_Specification& r) {
        extensions = r.extensions;
        tell_size = r.tell_size;
        load = r.load;
        unload = r.unload;
        on_gui = r.on_gui;
        set_default_params = r.set_default_params;
        param_size = r.param_size;
        creatable = r.creatable;
        asset_icon = r.icon;
        strcpy(name, r.name);
        runtime_data_size = r.runtime_data_size;
    }
    Hash_Set<Dynamic_String> extensions;
	std::function<size_t(str_ptr_t, void*)> tell_size = 0;
	std::function<byte*(byte*, str_ptr_t, void*)> load = 0;
	std::function<void(byte*)> unload = 0;
    std::function<void(void*)> on_gui = 0; 
    std::function<void(void*)> set_default_params = 0;
    size_t param_size;
    name_str_t name = "N/A";
    size_t runtime_data_size = 0;

    bool creatable;

    Icon_Type asset_icon = ICON_TYPE_FILE;
};

struct Free_Data_Block {
    data_index_t index;
    size_t size;
};

Gui_Window g_asset_manager = { true, "Asset Manager", "Assets" };
Gui_Window g_asset_inspector = { false, "Asset Inspector", "Assets" };

path_str_t g_current_dir;
path_str_t g_assets_dir;

Dynamic_Array<Asset> assets;
Dynamic_Array<byte> runtime_data;
Dynamic_Array<Free_Data_Block> free_data_blocks;

Dynamic_Array<Asset_Loader> loaders;

// Sorted by usage frequency
Dynamic_Array<asset_id_t> order_to_unload_assets;

u32 number_of_assets_in_use = 0;

Asset_Directory* g_root_directory = NULL;
Asset_Directory* g_selected_dir = NULL;

asset_id_t g_selected_asset = NULL_ASSET_ID;

Hash_Set<u32> taken_uids;

bool g_any_dir_rightclick = false;

// #Optimize
//     Should probably do bit shifting here...
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
    std::mt19937 rng;
    rng.seed((u32)time(NULL));
    u32 uid = rng() % UINT_MAX;
    while (taken_uids.find(uid) != taken_uids.end() && uid == (u32)-1) {
        uid = rng() % UINT_MAX;
    }
    taken_uids.emplace(uid);
    return uid;
}

bool is_uid_available(u32 uid) {
    return taken_uids.find(uid) == taken_uids.end() && uid != (u32)-1;
}

asset_id_t register_asset(str_ptr_t path, u32 uid = (u32)-1, void* params = NULL) {
    
    Asset a;
    a.is_garbage = false;

    path_str_t dir = "";
    Path::directory_of(path, dir);

    strcpy(a.path, path);
    Path::extension_of(path, a.extension);
    Path::name_with_extension(path, a.file_name);
    Path::name_without_extension(path, a.name);
    sprintf(a.meta_path, "%s/%s.%s", dir, a.name, "ap");
    
    bool any_loader = false;
    index_t i = 0;
    for (auto& loader : loaders) {
        if (loader.extensions.count(a.extension) > 0) {
            if (params) {
                a.load_param = params;
            } else {
                a.load_param = malloc(loader.param_size);
                loader.set_default_params(a.load_param);
            }

            a.loader = i;
            a.runtime_data_size = loader.runtime_data_size;

            a.icon = loader.asset_icon;

            strcpy(a.type, loader.name);

            any_loader = true;
            break;
        }
        i++;
    }

    if (!any_loader) {
        log_warn("Could not find loader for extension '{}'", a.extension);
        return NULL_ASSET_ID;
    }

    a.data_stream = &runtime_data;

    u32 index = (u32)-1;

    for (i = 0; i < assets.size(); i++) {
        if (assets[i].is_garbage) {
            assets[i] = std::move(a);
            index = (u32)i;
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

data_index_t load_to_memory(asset_id_t id) {
    u32 index = get_index(id);
    ap_assert(index < assets.size());
    Asset* asset = &assets[index];

    data_index_t data_index = runtime_data.size();

    auto& loader = loaders[asset->loader];

    size_t sz = loader.tell_size(asset->path, asset->load_param);

    bool need_resize = true;
    for (int i = (int)free_data_blocks.size() - 1; i >= 0; i--) {
        if (free_data_blocks[i].size >= sz) {
            data_index = free_data_blocks[i].index;
            need_resize = false;
            break;
        }
    }

    if (need_resize) {
        runtime_data.resize(runtime_data.size() + sz);
    }

    byte* stream = runtime_data.data() + data_index;

    byte* end = loader.load(stream, asset->path, asset->load_param);
    (void)end;
    ap_assert((uintptr_t)(end - stream) == (uintptr_t)sz, "Return value from load() mismatch with tell_size()");

    asset->in_memory = true;

    return data_index;
}

bool unload_from_memory(asset_id_t id) {
    u32 index = get_index(id);
    ap_assert(index < assets.size());
    Asset* asset = &assets[index];
    ap_assert(!asset->in_use, "Cannot unload an asset that's in use");

    free_data_blocks.push_back({ index, asset->runtime_data_size });

    auto& loader = loaders[asset->loader];
    loader.unload(runtime_data.data() + asset->data_index);

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

    free(asset.load_param);
    asset.load_param = NULL;

    asset.is_garbage = true;
}

Asset* begin_use(asset_id_t id) {
    u32 index = get_index(id);
    ap_assert(index < assets.size());
    number_of_assets_in_use++;

    Asset* asset = &assets[index];

    ap_assert(!asset->in_use, "Cannot begin use on an asset that's already in use");

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
    ImGui::Text(dir->real_path);
    ImGui::Separator();
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

        for (auto& loader : loaders) {
            if (loader.creatable && ImGui::MenuItem(loader.name)) {
                path_str_t new_path = "";
                sprintf(new_path, "%s/%s.%s", dir->real_path, create_name, (*loader.extensions.begin()).c_str());
                if (Path::create_file(new_path)) {
                    auto aid = register_asset(new_path);
                    if (aid != NULL_ASSET_ID) {
                        dir->assets.emplace(aid);
                    }
                } else {
                    log_error("Failed creating file at '{}'", new_path);
                }
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


                auto id = register_asset(new_path);
                if (id != NULL_ASSET_ID) {
                    dir->assets.emplace(id);
                }
            }
        });
    }

    if (ImGui::MenuItem("Delete")) {
        dir->parent_directory->delete_sub_dir(dir->real_path);
    }
}

void clear_assets() {
    for (auto& asset : assets) {
        ap_assert(!asset.in_use, "Asset is left in use when clearing asset system");
        if (asset.in_memory) unload_from_memory(asset.id);
    }
    assets.clear();
    if (g_root_directory) g_root_directory->clear();
    runtime_data.clear();
    taken_uids.clear();
}

bool validate(asset_id_t* paid) {
    asset_id_t& aid = *paid;

    if (aid == NULL_ASSET_ID) return false;

    auto idx = get_index(aid);
    auto uid = get_uid(aid);

    // If index is in range, asset isnt garbage and ids match - aid is valid.
    if (idx >= 0 && idx < assets.size() && !assets[idx].is_garbage && get_uid(assets[idx].id) == uid)
        return true;

    for (auto& asset : assets) {
        auto asset_idx = get_index(asset.id);
        auto asset_uid = get_uid(asset.id);

        if (asset.id == aid) {
            bool ret =  !asset.is_garbage;
            if (!ret) *paid = NULL_ASSET_ID;
            return ret;
        }

        // Unique id's match but not index; valid asset uid but
        // index has become incorrect when loading from disk
        if (uid == asset_uid && idx != asset_idx) {
            set_index(paid, asset_idx);
            bool ret =  !asset.is_garbage;
            if (!ret) *paid = NULL_ASSET_ID;
            return ret;
        }
    }

    *paid = NULL_ASSET_ID;
    return false;
}

Asset_Loader* get_loader(str_ptr_t ext) {
    for (auto& loader : loaders) {
        if (loader.extensions.count(ext) > 0) return &loader;
    }
    return NULL;
}

module_scope {
    module_function(void) on_load() {
        srand((u32)time(NULL));

        Graphics_Context* graphics = get_graphics();

        path_str_t folder_icon_path = "";
        path_str_t file_icon_path = "";

        graphics->set_clear_color(mz::color(.35f, .1f, .65f, 1.f));

        register_gui_window(&g_asset_manager);
        register_gui_window(&g_asset_inspector);

        strcpy(g_current_dir, get_executable_directory());
    }

    module_function(void) on_play_begin() {
        
    }

    module_function(void) on_play_stop() {
        
    }

    module_function(void) save_to_disk(str_ptr_t dir) {
        (void)dir;

        sprintf(g_assets_dir, "%s/assets", dir);

        // Clean up all previous meta files
        Path::iterate_directories(g_assets_dir, [](str_ptr_t entry){
            path_str_t ext = "";
            Path::extension_of(entry, ext);

            if (strcmp(ext, "ap") == 0) {
                Path::remove(entry);
            }
        }, true);

        // Write all meta files
        for (auto& asset : assets) {
            if (asset.is_garbage) continue;
            Binary_Archive params_archive(asset.meta_path);

            auto& loader = loaders[asset.loader];

            params_archive.write<u32>("uid", get_uid(asset.id));
            params_archive.write("params", asset.load_param, loader.param_size);

            params_archive.flush();
        }

    }

    module_function(void) load_from_disk(str_ptr_t dir) {
        
        clear_assets();

        sprintf(g_assets_dir, "%s/assets", dir);

        if (g_root_directory) delete g_root_directory;
        g_root_directory = new Asset_Directory(NULL, g_assets_dir);
        g_selected_dir = g_root_directory;

        sprintf(g_assets_dir, "%s/assets", dir);
        if (!Path::exists(g_assets_dir)) Path::create_directory(g_assets_dir);

        Path::iterate_directories(g_assets_dir, [](str_ptr_t entry) {
            if (Path::is_directory(entry)) {
                path_str_t entry_parent_dir = "";
                Path::directory_of(entry, entry_parent_dir);
                g_root_directory->traverse([&](Asset_Directory& dir) {
                    if (Path::equals(dir.real_path, entry_parent_dir)) {
                        dir.sub_directories.push_back(new Asset_Directory(&dir, entry));
                    }
                });
            } else if (Path::is_file(entry)) {
                path_str_t ext = "";
                Path::extension_of(entry, ext);
                if (strcmp(ext, "ap") != 0) {
                    auto* loader = get_loader(ext);
                    if (!loader) {
                        log_error("Missing loader for asset at '{}'", entry);
                        return;
                    }
                    path_str_t asset_dir = "";
                    path_str_t meta_file = "";
                    path_str_t asset_name = "";
                    Path::directory_of(entry, asset_dir);
                    Path::name_without_extension(entry, asset_name);
                    sprintf(meta_file, "%s/%s.ap", asset_dir, asset_name);
                    
                    if (!Path::exists(meta_file)) {
                        register_asset(entry);
                    } else {
                        Binary_Archive meta_archive(meta_file);

                        if (meta_archive.is_valid_id("uid") && meta_archive.is_valid_id("params")) {
                            u32 uid = meta_archive.read<u32>("uid");
                            size_t params_size = 0;
                            void* params = meta_archive.read("params", &params_size);
                            if (params_size != loader->param_size) {
                                params = NULL;
                            } else {
                                // Copy it, because the memory is freed when the archive
                                // is freed
                                void* params_copy = malloc(params_size);
                                memcpy(params_copy, params, params_size);
                                params = params_copy;
                            }
                            register_asset(entry, uid, params);
                        } else {
                            register_asset(entry);
                        }
                    }
                }
            }
        }, true);

        g_root_directory->traverse([](Asset_Directory& dir) {
            for (auto& asset : assets) {
                path_str_t asset_dir = "";
                Path::directory_of(asset.path, asset_dir);
                if (Path::equals(asset_dir, dir.real_path)) {
                    dir.assets.emplace(asset.id);
                }
            }
        });
    }

    module_function(void) on_unload() {
        clear_assets();
        if (g_root_directory) delete g_root_directory;

        unregister_gui_window(&g_asset_manager);
        unregister_gui_window(&g_asset_inspector);
    }

    module_function(void) on_update(float delta) {
        (void)delta;


    }

    module_function(void) on_render() {
        
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

        
        if (ImGui::BeginPopupContextItem(dir.real_path)) {
            g_any_dir_rightclick = true;
            do_asset_manager_menu_gui(&dir);
            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropTarget()) {
            auto* p = ImGui::AcceptDragDropPayload("asset");
            if (p) {
                auto payload = (Gui_Payload*)p->Data;
                auto asset_id = (asset_id_t)(uintptr_t)payload->value;
                auto* prev_dir = (Asset_Directory*)payload->home;

                Asset& asset = assets[get_index(asset_id)];

                path_str_t prev_path = "";
                path_str_t prev_meta_path = "";
                strcpy(prev_path, asset.path);
                strcpy(prev_meta_path, asset.meta_path);
                sprintf(asset.path, "%s/%s", dir.real_path, asset.file_name);

                path_str_t asset_dir = "";
                Path::directory_of(asset.path, asset_dir);
                sprintf(asset.meta_path, "%s/%s.ap", asset_dir, asset.name);

                if (Path::copy(prev_path, asset.path).value() == 0) {
                    dir.assets.emplace(asset_id);
                    Path::copy(prev_meta_path, asset.meta_path);

                    if (Path::remove(prev_path).value() == 0) {
                        prev_dir->assets.erase(asset_id);
                        Path::remove(prev_meta_path);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        

        if (node_open) {	
            for (auto sub : dir.sub_directories) {
                do_dir_gui(*sub, ++depth);
            }
            for (auto asset_id : dir.assets) {
                auto& asset = assets[get_index(asset_id)];

                ImGui::Icon(asset.icon, { 16, 16 });
                vline_end = ImGui::GetItemRectMin();
                vline_end.y += 8;
                f32 x = vline_start.x > vline_end.x ? vline_end.x : vline_start.x;
                draw_list->AddLine(vline_end, { x, vline_end.y }, ImGui::GetColorU32(ImGuiCol_Text));
                ImGui::SameLine();
                if (ImGui::Selectable(asset.name, g_selected_asset == asset.id)) {
                    g_selected_asset = asset.id;
                }
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
                    Gui_Payload payload;
                    payload.home = &dir;
                    payload.value = (void*)(uintptr_t)asset_id;
                    ImGui::SetDragDropPayload("asset", &payload, sizeof(Gui_Payload));
                    ImGui::Text(asset.name);
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

    module_function(void) on_gui() {
        ImGui::DoGuiWindow(&g_asset_manager, [&]() {

            ImGui::BeginMenuBar();

            do_asset_manager_menu_gui(g_selected_dir ? g_selected_dir : g_root_directory);

            ImGui::EndMenuBar();

            do_dir_gui(*g_root_directory);

            if (!g_any_dir_rightclick && ImGui::BeginPopupContextWindow("ooga booga")) {
                do_asset_manager_menu_gui(g_root_directory);
                ImGui::EndPopup();
            }
            g_any_dir_rightclick = false;

        }, ImGuiWindowFlags_MenuBar);

        ImGui::DoGuiWindow(&g_asset_inspector, [&]() {

            if (validate(&g_selected_asset)) {
                Asset& asset = assets[get_index(g_selected_asset)];
                auto& loader = loaders[asset.loader];

                ImGui::Text("%s (%s)", asset.file_name, loader.name);
                ImGui::Separator();
                loader.on_gui(asset.load_param);

                ImGui::Separator();

                if (ImGui::Button("Reload") && asset.in_memory) {
                    unload_from_memory(g_selected_asset);
                    asset.data_index = load_to_memory(g_selected_asset);
                }
            }

        });
    }

    module_function(void*) get_function_library() {

        static Asset_Manager_Function_Library lib;

        lib.begin_use = begin_use;
        lib.end_use = end_use;
        lib.validate = validate;
        lib.view = [](asset_id_t aid) {
            if (validate(&aid)) {
                return &assets[get_index(aid)];
            } else {
                return (Asset*)nullptr;
            }
        };
        lib.register_loader = [](const Asset_Loader_Specification& spec) {
            auto assure = [](bool val, str_ptr_t err) {
                if (!val) log_error("Asset loader invalid: {}", err);
                return val;
            };

            if (!assure(spec.extensions.size() > 0, "Asset loader must handle at least one extension")) return;
            if (!assure((bool)spec.tell_size, "Asset loader must have valid tell_size() function")) return;
            if (!assure((bool)spec.load, "Asset loader must have valid load() function")) return;
            if (!assure((bool)spec.unload, "Asset loader must have valid unload() function")) return;
            if (!assure((bool)spec.set_default_params, "Asset loader must have valid set_default_params() function")) return;
            if (!assure(spec.param_size > 0, "Asset loader param size must be > 0")) return;
            if (!assure(spec.runtime_data_size > 0, "Asset loader runtime data size must be > 0")) return;
            
            for (auto& loader : loaders) {
                for (auto& ext : loader.extensions) {
                    if (spec.extensions.count(ext) > 0) {
                        log_warn("Loader for extension {} already exists, undefined behaviour follows...", ext);
                    }
                }
            }

            loaders.emplace_back(spec);
        };
        lib.unregister_loader = [](str_ptr_t name) {
            for (int i = (int)loaders.size() - 1; i >= 0; i--) {
                if (strcmp(loaders[i].name, name) == 0) {
                    loaders.erase(loaders.begin() + i);
                }
            }
        };

        return &lib;
    }
}