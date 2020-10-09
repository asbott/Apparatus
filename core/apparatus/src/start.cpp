#include "pch.h"

#include <iostream>
#include <ostream>

#include "start.h"

#include "dependencies.h"

#include "graphics/graphics_api.h"
#include "graphics/opengl45.h"
#include "graphics/directx11.h"

#include "image_import.h"

#include "os.h"

void graphics_debug_callback(graphics_enum_t severity, const char* msg, const char* fn) {
    char output_msg[4096] = "";
    sprintf(output_msg, "Graphics debug message (%s):\n%s\nIn '%s'", 
        get_graphics_enum_string(severity), msg, fn);
    switch (severity) {
    case G_DEBUG_MESSAGE_SEVERITY_NOTIFY:   log_trace(output_msg);    break;
    case G_DEBUG_MESSAGE_SEVERITY_WARNING:  log_warn(output_msg);     break;
    case G_DEBUG_MESSAGE_SEVERITY_ERROR:    log_error(output_msg);    break;
    case G_DEBUG_MESSAGE_SEVERITY_CRITICAL: log_critical(output_msg); _AP_BREAK; break;
    }
}

using namespace mz;

Graphics_Context* g_graphics;
bool g_running = true;

Thread_Server g_thread_server;
thread_id_t g_graphics_thread;
entt::registry g_reg;

Dynamic_Array<Module*> g_modules;

Hash_Set<Gui_Window*> gui_windows;
Hash_Set<Gui_Popup*> gui_popups;

Static_Array<graphics_id_t, ICON_TYPE_COUNT> icons;

Gui_Window entity_inspector = { true, "Entity Inspector" };
Gui_Window scene_inspector = { true, "Scene Inspector" };
Gui_Window module_manager = { false, "Modules Manager" };
Gui_Window log_window = { false, "Log" };

Gui_Popup add_component_popup;
Gui_Popup manage_component_popup;

Log_Context log_context;

Game_Input game_input_state;

Hash_Set<entt::entity> g_selected_entities;

namespace ImGui {
	void Icon(Icon_Type icon, mz::ivec2 size) {
        auto native = g_graphics->get_native_texture_handle(icons[icon]);
        ImGui::Image(native, size);
    }
	bool IconButton(Icon_Type icon, mz::ivec2 size) {
        auto native = g_graphics->get_native_texture_handle(icons[icon]);
        return ImGui::ImageButton(native, size);
    }
}

void quit() {
    g_running = false;
}

Game_Input* game_input() {
    return &game_input_state;
}

entt::registry& get_entity_registry() {
    return g_reg;
}

Module* get_module(name_str_t str_id) {
    for (auto* mod : g_modules) {
        if (strcmp(mod->str_id, str_id) == 0) return mod;
    }
    return NULL;
}

void register_gui_window(Gui_Window* wnd) {
    gui_windows.emplace(wnd);
}

void unregister_gui_window(Gui_Window* wnd) {
    gui_windows.erase(wnd);
}

void register_gui_popup(Gui_Popup* pop) {
    gui_popups.emplace(pop);
}

void unregister_gui_popup(Gui_Popup* pop) {
    gui_popups.erase(pop);
}

void load_icon(Icon_Type type, str_ptr_t dir, str_ptr_t offset_path) {
    path_str_t icon_path = "";

    sprintf(icon_path, "%s/%s", dir, offset_path);

    mz::ivec3 icon_size;
    auto img = load_image_from_file(icon_path, &icon_size.x, &icon_size.y, &icon_size.z, 4);
    ap_assert(img != NULL, "Failed loading icon at {}: {}", offset_path, get_failure_reason());

    graphics_id_t icon = g_graphics->make_texture(G_BUFFER_USAGE_STATIC_WRITE);

    g_graphics->set_texture_filtering(icon, G_MIN_FILTER_NEAREST, G_MAG_FILTER_NEAREST);

    g_graphics->set_texture_wrapping(icon, G_WRAP_CLAMP_TO_BORDER);

    g_graphics->set_texture_data(icon, img, icon_size, G_TEXTURE_FORMAT_RGBA);

    icons[type] = icon;
}

Thread_Server& get_thread_server() {
    return g_thread_server;
}

thread_id_t get_graphics_thread() {
    return g_graphics_thread;
}

void register_module(name_str_t mod_name) {

    path_str_t mod_path = "";
    path_str_t mod_path_new = "";
    sprintf(mod_path, "%s/../runtime/%s_used.dll", get_executable_directory(), mod_name);
    sprintf(mod_path_new, "%s/../runtime/%s.dll", get_executable_directory(), mod_name);

    auto err = Path::copy(mod_path_new, mod_path);
    ap_assert(err.value() == 0, "Copy fail: {}", err.message());

    g_modules.push_back(new Module(mod_path, mod_path_new, mod_name));
}

void select_entity(entt::entity entity) {
    if (!ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && !ImGui::IsKeyDown(AP_KEY_RIGHT_CONTROL)) {
        deselect_all_entities();    
    }
    g_selected_entities.emplace(entity);
}
void deselect_entity(entt::entity entity) {
    g_selected_entities.erase(entity);
}
void deselect_all_entities() {
    g_selected_entities.clear();
}
bool is_entity_selected(entt::entity entity) {
    return g_selected_entities.count(entity) > 0;
}
const Hash_Set<entt::entity>& get_selected_entities() {
    return g_selected_entities;
}

void to_file(entt::registry& reg, str_ptr_t path) {
    std::stringstream ss;

    reg.each([&reg, &path, &ss](entt::entity entity){
        if (!reg.has<Entity_Info>(entity)) return;
        auto& entity_info = reg.get<Entity_Info>(entity);
        ss << "$entity_begin\n";
        ss << "$name " << entity_info.name << "\n";
        for (auto mod : g_modules) {
            if (!mod->is_loaded) continue;
            for (auto component_id : mod->get_component_ids()) {
                if (auto* data = (byte*)mod->get_component(component_id, reg, entity)) {
                    ss << "$component_begin\n";
                    const auto& info = mod->get_component_info(component_id);
                    ss << "$name " << info->name << "\n";
                    for (auto& prop : info->properties) {
                        ss << "$property_begin\n";
                        ss << "$name " << prop.name << "\n";
                        ss << "$bytes ";
                        for (int i = 0; i < prop.size; i++) {
                            ss << (int)data[prop.offset + i] << " ";
                        }
                        ss << "\n";
                        ss << "$property_end\n";
                    }
                    ss << "$component_end\n";
                }
            }
        }
        ss << "$entity_end\n";

    });

    Dynamic_String dstring = ss.str();
    str_ptr_t str = dstring.c_str();

    Path::write_bytes(path, (byte*)str, strlen(str) + 1);
}

void from_file(entt::registry& reg, str_ptr_t path) {
    File_Info file_info;
    Path::get_info(path, &file_info);

    char* buffer = (char*)malloc(file_info.size);

    Path::read_all_bytes(path, (byte*)buffer, file_info.size);

    constexpr u32 LEVEL_NONE = 0;
    constexpr u32 LEVEL_ENTITY = 1;
    constexpr u32 LEVEL_COMPONENT = 2;
    constexpr u32 LEVEL_PROPERTY = 3;

    constexpr u32 ACTION_NONE = 0;
    constexpr u32 ACTION_SETTING_NAME = 1;
    constexpr u32 ACTION_SETTING_BYTES = 2;

    u32 level = LEVEL_NONE;
    u32 action = ACTION_NONE;

    entt::entity entity = entt::null;
    Dynamic_Array<byte> bytes;
    uintptr_t component_id = 0;
    Module* component_module = NULL;

    int word_start = 0;
    name_str_t word = "";
    for (int i = 0; i < file_info.size; i++) {
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r\n' || buffer[i] == '\r') {
            memset(word, 0, sizeof(name_str_t));
            memcpy(word, buffer + word_start, i - word_start);
            i++;
            
            if (level != LEVEL_NONE) {
                if (strcmp(word, "$name") == 0) {
                    action = ACTION_SETTING_NAME;
                    word_start = i;
                    continue;
                }
                if (strcmp(word, "$bytes") == 0) {
                    action = ACTION_SETTING_BYTES;
                    word_start = i;
                    continue;
                }
            }

            if (level == LEVEL_NONE) {
                if (strcmp(word, "$entity_begin") == 0) {
                    level++;
                    entity = reg.create();
                    reg.emplace<Entity_Info>(entity).id = entity;
                }

            } else if (level == LEVEL_ENTITY) {
                if (strcmp(word, "$component_begin") == 0) level++;
                if (strcmp(word, "$entity_end") == 0) level--;

                if (action == ACTION_SETTING_NAME) {
                    strcpy(reg.get<Entity_Info>(entity).name, word);
                    action = ACTION_NONE;
                }

            } else if (level == LEVEL_COMPONENT) {
                if (strcmp(word, "$property_begin") == 0) level++;
                if (strcmp(word, "$component_end") == 0) {
                    if (component_id) {
                        const auto& info = component_module->get_component_info(component_id);
                        byte* comp = (byte*)component_module->create_component(component_id, reg, entity);
                        size_t comp_size = 0;
                        for (auto& prop : info->properties) comp_size += prop.size;
                        if (comp_size == bytes.size()) {
                            size_t bytes_consumed = 0;
                            for (auto& prop : info->properties) {
                                memcpy(comp + prop.offset, &bytes[bytes_consumed], prop.size);
                                bytes_consumed += prop.size;
                            }
                        } else {
                            log_warn("Data mismatch when loading component {} on entity {}", 
                            info->name, reg.get<Entity_Info>(entity).name);
                        }
                        bytes.clear();
                        component_id = 0;
                        component_module = NULL;
                    }
                    level--;
                }

                if (action == ACTION_SETTING_NAME) {
                    action = ACTION_NONE;
                    for (auto mod : g_modules) {
                        if (!mod->is_loaded) continue;
                        if (auto id = mod->get_component_id(word)) {
                            component_module = mod;
                            component_id = id;
                            break;
                        }
                    }
                }
                
            } else if (level == LEVEL_PROPERTY) {
                if (strcmp(word, "$property_end") == 0) level--;

                if (action == ACTION_SETTING_BYTES) {
                    action = ACTION_NONE;
                    bytes.push_back((byte)std::stoi(word));
                    str_t<4> byte_str = "";
                    int byte_start = i;
                    while (buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\r\n') {
                        if (buffer[i] == ' ') {
                            memset(byte_str, 0, sizeof(byte_str));
                            memcpy(byte_str, buffer + byte_start, i - byte_start);
                            bytes.push_back((byte)std::stoi(byte_str));
                            byte_start = i + 1;
                        }
                        i++;
                    }
                    word_start = i++;
                }
            }
            word_start = i;
        }
    }

    free(buffer);
}

int start(int argc, char** argv) {
    (void)argc;

    srand((u32)time(NULL));

    ___set_executable_path(argv[0]);

    std::ofstream log_stream;
    log_stream.open("log_output.txt");
    init_logger(log_stream, &log_context);
    init_dependencies();

    set_logger_level(spdlog::level::trace);

    auto exe = get_executable_path();
    char ext[1024] = "";
    char name_with[1024] = "";
    char name_without[1024] = "";
    Path::extension_of(exe, ext);
    Path::name_with_extension(exe, name_with);
    Path::name_without_extension(exe, name_without);

    log_trace("Exe path: {}\nExe dir:  {}\nExe ext:  {}\nExe name: {}, {}",
        exe, get_executable_directory(), ext, name_with, name_without);

    path_str_t project_dir = "";
    sprintf(project_dir, "%s/../..", get_executable_directory());
    Path::to_absolute(project_dir, project_dir);

    path_str_t ecs_path = "";
    sprintf(ecs_path, "%s/ecs", project_dir);

    path_str_t essential_dir = "";
    sprintf(essential_dir, "%s/../../essential", get_executable_directory());

    log_trace("ecs_path: {}", ecs_path);

    register_gui_window(&scene_inspector);
    register_gui_window(&entity_inspector);
    register_gui_window(&module_manager);
    register_gui_window(&log_window);

    add_component_popup.is_modal = true;
    strcpy(add_component_popup.str_id, "Add Component");
    add_component_popup.fn = [](){
        for (auto* mod : g_modules)  {
            if (!mod->is_loaded) continue;
            if (ImGui::CollapsingHeader(mod->str_id)) {
                const auto& ids = mod->get_component_ids();
                for (auto id : ids) {
                    const auto& info = mod->get_component_info(id);
                    if (g_selected_entities.size() == 1) {
                        ImGuiSelectableFlags flags = ImGuiSelectableFlags_None;
                        if (mod->has_component(id, g_reg, *g_selected_entities.begin())) 
                            flags |= ImGuiSelectableFlags_Disabled;
                        if (ImGui::Selectable(info->name.c_str(), false, flags)) {
                            mod->create_component(id, g_reg, *g_selected_entities.begin());
                            ImGui::CloseCurrentPopup();
                        }
                    } else {
                        if (ImGui::Selectable(info->name.c_str())) {
                            for (auto selected_entity : g_selected_entities) {
                                if (mod->get_component(id, g_reg, selected_entity) == NULL)
                                    mod->create_component(id, g_reg, selected_entity);   
                            }
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
            }
        }

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
    };

    manage_component_popup.is_modal = false;
    strcpy(manage_component_popup.str_id, "");
    manage_component_popup.fn = []() {
        if (ImGui::MenuItem("Remove")) {
            strcpy(manage_component_popup.return_value, "Remove");
            if (manage_component_popup.done_fn) manage_component_popup.done_fn();
        }
        
    };  

    register_gui_popup(&add_component_popup);
    register_gui_popup(&manage_component_popup);
        
    g_graphics_thread = g_thread_server.make_thread();

    g_graphics = new Graphics<Default_Api>();
    g_graphics->debug_callback = graphics_debug_callback;
    g_graphics->init(true, &g_thread_server, g_graphics_thread);

    register_module("test_module");
    register_module("ecs_2d_renderer");
    register_module("asset_manager");
    register_module("2d_physics");

    for (auto* mod : g_modules) {
        log_trace("Loading module '{}'", mod->str_id);
        if (mod->load()) {
            log_info("Successfully loaded module '{}'", mod->str_id);
        } else {
            log_error("Failed loading module '{}'", mod->str_id);
        }

        if (mod->on_load) mod->on_load(g_graphics);
        if (mod->init) mod->init();
    }

    if (Path::exists(ecs_path)) {
        from_file(g_reg, ecs_path);
    }

    bool is_playing = false;

    path_str_t font_path = "";
    sprintf(font_path, "%s/input_mono.ttf", essential_dir);
    ImFontConfig cfg;
    cfg.OversampleH = 8;
    cfg.OversampleV = 8;
    cfg.RasterizerMultiply = 1.f;
    auto* font = ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 16, &cfg);
    ImGui::GetIO().FontDefault = font;

    load_icon(ICON_TYPE_STOP,    essential_dir, "stop_icon.png");
    load_icon(ICON_TYPE_PLAY,    essential_dir, "play_icon.png");
    load_icon(ICON_TYPE_OPTIONS, essential_dir, "options_icon.png");
    load_icon(ICON_TYPE_TEXTURE, essential_dir, "texture_icon.png");
    load_icon(ICON_TYPE_FOLDER,  essential_dir, "vendor/folder_icon.png");
    load_icon(ICON_TYPE_FILE,    essential_dir, "file_icon.png");

    for (auto* mod : g_modules) if (mod->load_from_disk) mod->load_from_disk(project_dir);

    while (g_running) {
        
        auto* windows = g_graphics->get_windows_context();
        auto* wnd = windows->main_window_handle;

        g_graphics->update_imgui();

        g_thread_server.wait_for_thread(g_graphics_thread);
        ImGui::DockSpaceOverViewport();

        // Clear the color buffer
        g_graphics->clear(G_COLOR_BUFFER_BIT);

        float delta = (f32)windows->window_info[wnd].delta_time;

        
        if (is_playing) {
            for (auto* mod : g_modules) if (mod->on_update) mod->on_update(delta);
        }

        log_context.do_gui(&log_window);

        static Dynamic_Array<Module*> to_reload;
        static Dynamic_Array<Module*> to_unload;
        static Dynamic_Array<Module*> to_load;

        ImGui::DoGuiWindow(&module_manager, []() {
            static Module* selected_module = NULL;
            
            if (ImGui::RBeginCombo("Module", selected_module ? selected_module->str_id : "<none>")) {
                for (auto mod : g_modules) {
                    if (ImGui::Selectable(mod->str_id, mod == selected_module)) {
                        selected_module = mod;
                    }
                }   

                ImGui::REndCombo();
            }

            ImGui::Separator();
            if (selected_module) {
                ImGui::Text("Module: %s", selected_module->str_id);
                ImGui::Text("Has function '%s': %s", "on_load",         selected_module->on_load         ? "yes" : "no");
                ImGui::Text("Has function '%s': %s", "on_unload",       selected_module->on_unload       ? "yes" : "no");
                ImGui::Text("Has function '%s': %s", "on_update",       selected_module->on_update       ? "yes" : "no");
                ImGui::Text("Has function '%s': %s", "on_render",       selected_module->on_render       ? "yes" : "no");
                ImGui::Text("Has function '%s': %s", "on_gui",          selected_module->on_gui          ? "yes" : "no");
                ImGui::Text("Has function '%s': %s", "on_play_begin",   selected_module->on_play_begin   ? "yes" : "no");
                ImGui::Text("Has function '%s': %s", "on_play_stop",    selected_module->on_play_stop    ? "yes" : "no");
                ImGui::Text("Has function '%s': %s", "save_to_disk",    selected_module->save_to_disk    ? "yes" : "no");
                ImGui::Text("Has function '%s': %s", "load_from_disk",  selected_module->load_from_disk  ? "yes" : "no");
                ImGui::Text("Has function '%s': %s", "_request",        selected_module->_request        ? "yes" : "no");

                if (selected_module->is_loaded) {
                    ImGui::Spacing();
                    ImGui::Text("Number of components: %i", selected_module->get_component_ids().size());
                }

                ImGui::Spacing();
                ImGui::Text("Is loaded: %s", selected_module->is_loaded ? "yes" : "no");

                if (selected_module->is_loaded) {
                    if (ImGui::Button("Reload")) {
                        to_reload.push_back(selected_module);
                    }
                    if (ImGui::Button("Unload")) {
                        to_unload.push_back(selected_module);
                    }
                } else {
                    if (ImGui::Button("Load")) {
                        to_load.push_back(selected_module);
                    }
                }
            }
        });

        for (auto* mod : g_modules) if (mod->on_render) mod->on_render(g_graphics);

        ImGui::UseGraphicsContext(g_graphics);
        for (auto* mod : g_modules) if (mod->on_gui) mod->on_gui(g_graphics, ImGui::GetCurrentContext());    
        
        static f32 bar_width = (f32)windows->window_info[wnd].size.width;
        static f32 bar_height = 32.f;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { bar_width, bar_height });
        ImGui::BeginMainMenuBar();
        ImGui::PopStyleVar();
        bar_width = ImGui::GetWindowWidth();

        if (ImGui::BeginMenu("Windows")) {
            for (auto* gui_wnd : gui_windows) {
                ImGui::MenuItem(gui_wnd->name, NULL, &gui_wnd->open);
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        bool want_invoke_on_play_begin = false;
        bool want_invoke_on_play_stop   = false;

        ImGui::Indent(ImGui::GetWindowWidth() / 2.f - 16);
        bool toggle_play = ImGui::IconButton(is_playing ? ICON_TYPE_STOP : ICON_TYPE_PLAY, { 32, 32 });
        bar_height = ImGui::GetItemRectSize().y / 2.f - ImGui::GetStyle().FramePadding.y * 2 -3;
        if (toggle_play) {
            if (!is_playing) {
                to_file(g_reg, ecs_path);
                for (auto mod : g_modules) if (mod->save_to_disk) mod->save_to_disk(project_dir);
                g_thread_server.wait_for_thread(g_graphics_thread);
                want_invoke_on_play_begin = true;
            } else {
                g_reg.clear();
                from_file(g_reg, ecs_path);
                for (auto mod : g_modules) if (mod->load_from_disk) mod->load_from_disk(project_dir);
                g_thread_server.wait_for_thread(g_graphics_thread);
                want_invoke_on_play_stop = true;
            }

            is_playing = !is_playing;
        }
 
        ImGui::EndMainMenuBar();

        ImGui::DoGuiWindow(&scene_inspector, [&]() {
            ImGui::BeginMenuBar();
            if (ImGui::BeginMenu("Create")) {

                static str_t<128> buf;
                ImGui::RInputText("Name", buf, sizeof(buf));

                if (ImGui::Button("Create")) {
                    auto entity = g_reg.create();
                    g_reg.emplace<Entity_Info>(entity);
                    g_reg.get<Entity_Info>(entity).id = entity;
                    strcpy(g_reg.get<Entity_Info>(entity).name, buf);
                    strcpy(buf, "");
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();

            bool any_clicked = false;
            g_reg.view<Entity_Info>().each([&](entt::entity entity, Entity_Info& entity_info) {
                char label[256] = "";
                sprintf(label, "%s##%i", entity_info.name, entity_info.id);

                if (ImGui::Selectable(entity_info.name, g_selected_entities.count(entity) > 0)) {
                    if (!ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && !ImGui::IsKeyDown(AP_KEY_RIGHT_CONTROL)) {
                        deselect_all_entities();
                    }
                    select_entity(entity);
                    any_clicked = true;
                }
            });

            if (ImGui::IsMouseClicked(0) && !any_clicked && ImGui::IsWindowHovered()) {
                deselect_all_entities();
            }
        }, ImGuiWindowFlags_MenuBar);

        
        ImGui::DoGuiWindow(&entity_inspector, [&]() {
            for (auto selected_entity : g_selected_entities) {
                if (g_reg.valid(selected_entity)) {
                    for (auto* mod : g_modules) {
                        if (!mod->is_loaded) continue;
                        const auto& ids = mod->get_component_ids();
                        for (auto id : ids) {
                            if (void* comp = mod->get_component(id, g_reg, selected_entity)) {
                                const auto& info = mod->get_component_info(id);
                                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;// | ImGuiTreeNodeFlags_Bullet;
                                bool component_opened = ImGui::TreeNodeEx(info->name.c_str(), flags);
                                
                                ImGui::SameLine(ImGui::GetWindowWidth() - 30);
                                
                                ImGui::PushID(comp);
                                if (ImGui::IconButton(ICON_TYPE_OPTIONS, { 16, 16 })) {
                                    
                                    manage_component_popup.should_open = true;
                                    manage_component_popup.done_fn = [mod, id, selected_entity]() {
                                        if (strcmp(manage_component_popup.return_value, "Remove") == 0) {
                                            mod->remove_component(id, g_reg, selected_entity);
                                        }
                                    };
                                }
                                ImGui::PopID();

                                if (component_opened) {
                                    if (info->has_custom_gui && info->properties.size() > 0) {
                                        info->properties[0].on_gui(comp, ImGui::GetCurrentContext());
                                    } else {
                                        for (const auto& prop : info->properties) {
                                            prop.on_gui((byte*)comp + prop.offset, ImGui::GetCurrentContext());
                                        }
                                    }

                                    ImGui::TreePop();
                                }
                                ImGui::Separator();
                            }
                        }
                    }

                    if (ImGui::MenuItem("Add Component")) {
                        add_component_popup.should_open = true;
                    }
                }
            }
            if (g_selected_entities.size() == 0) {
                ImGui::Text("No entity selected");
            }
        });

        for (auto* popup : gui_popups) {
            if (popup->should_open) {
                popup->should_open = false;
                ImGui::OpenPopup(popup->str_id);
            }
            bool show = popup->is_modal 
                ? ImGui::BeginPopupModal(popup->str_id)
                : ImGui::BeginPopup(popup->str_id);

            if (show) {
                popup->fn();
                ImGui::EndPopup();
            }
        }

        g_graphics->render_imgui();

        if (want_invoke_on_play_begin) for (auto* mod : g_modules) if (mod->on_play_begin) mod->on_play_begin();
        if (want_invoke_on_play_stop)  for (auto* mod : g_modules) if (mod->on_play_stop)  mod->on_play_stop();

        // Swap the buffers in swap chain
        windows->swap_buffers(wnd);
        
        update_dependencies();

        for (auto* mod : to_reload) {
            if (mod->on_unload) mod->on_unload(g_graphics);
            if (mod->reload()) {
                if (mod->on_load) mod->on_load(g_graphics);
                ap_assert(mod->init, "init was not found in module");
                mod->init();
            }
        }
        to_reload.clear();

        for (auto* mod : to_load) {
            if (mod->load()) {
                if (mod->on_load) mod->on_load(g_graphics);
                ap_assert(mod->init, "init was not found in module");
                mod->init();
            }
        }
        to_load.clear();

        for (auto* mod : to_unload) {
            if (mod->is_loaded) {
                for (auto comp_id : mod->get_component_ids()) {
                    g_reg.each([mod, comp_id](entt::entity entity) {
                        if (mod->get_component(comp_id, g_reg, entity)) {
                            mod->remove_component(comp_id, g_reg, entity);
                        }
                    });
                }
            }
            if (mod->on_unload) mod->on_unload(g_graphics);
            if (mod->unload()) {
                
            }
        }
        to_unload.clear();
    }

    shutdown_dependencies();

    log_stream.close();

    return 0;
}