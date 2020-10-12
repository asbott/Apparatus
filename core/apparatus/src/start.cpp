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
Gui_Window style_editor = { false, "Style Editor" };

Gui_Popup add_component_popup;
Gui_Popup manage_component_popup;

Log_Context log_context;

Game_Input game_input_state;

Hash_Set<entt::entity> g_selected_entities;

bool g_is_playing = false;

path_str_t g_user_dir = "";

namespace ImGui {
	void Icon(Icon_Type icon, mz::ivec2 size) {
        auto native = g_graphics->get_native_texture_handle(icons[icon]);
        ImGui::Image(native, size);
    }
	bool IconButton(Icon_Type icon, mz::ivec2 size, const mz::color& bgr_color) {
        auto native = g_graphics->get_native_texture_handle(icons[icon]);
        if (bgr_color != mz::color(0)) ImGui::PushStyleColor(ImGuiCol_Button, bgr_color);
        bool pressed = ImGui::ImageButton(native, size, {0,0}, {1,1}, -1);
        if (bgr_color != mz::color(0)) ImGui::PopStyleColor();
        return pressed;
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
bool is_any_entity_selected() {
    return g_selected_entities.size() > 0;
}
const Hash_Set<entt::entity>& get_selected_entities() {
    return g_selected_entities;
}

str_ptr_t get_user_directory() {
    return g_user_dir;   
}

bool is_playing() {
    return g_is_playing;
}

void to_file(entt::registry& reg, str_ptr_t dir_path) {
    
    Path::create_directory(dir_path);
    reg.view<Entity_Info>().each([&reg, &dir_path](entt::entity entity, Entity_Info& entity_info) {
        path_str_t file_path = "";
        sprintf(file_path, "%s/%s", dir_path, entity_info.name);
        Binary_Archive entity_archive(file_path);

        entity_archive.write("name", entity_info.name);
        size_t ncomponents = 0;
        for (auto* mod : g_modules) {
            if (!mod->is_loaded) continue;
            auto& ids = mod->get_component_ids();
            for (auto comp_id : ids) {
                if (!mod->has_component(comp_id, reg, entity)) continue;
                ncomponents++;
                const auto& info = mod->get_component_info(comp_id);
                path_str_t comp_file_path = "";
                sprintf(comp_file_path, "%s/%s.%s", dir_path, entity_info.name, info->name.c_str());

                str_t<sizeof("component_") + 1> id = "";
                sprintf(id, "component_%llu", ncomponents);
                entity_archive.write(id, comp_file_path);

                Binary_Archive comp_archive(comp_file_path);
                for (const auto& prop : info->properties) {
                    entity_name_t prop_name = "";
                    strcpy(prop_name, prop.name.c_str());
                    byte* comp = (byte*) mod->get_component(comp_id, reg, entity);
                    comp_archive.write(prop_name, comp + prop.offset, prop.size);
                }
                comp_archive.flush();
            }
        }
        entity_archive.write("ncomponents", ncomponents);

        entity_archive.flush();
    });
}

void from_file(entt::registry& reg, str_ptr_t dir_path) {
    Path::iterate_directories(dir_path, [&reg](str_ptr_t entry) {
        if (Path::is_file(entry) && !Path::has_extension(entry)) {
            entity_name_t entity_name = "";
            Path::name_without_extension(entry, entity_name);

            entt::entity entity = reg.create();
            auto& entity_info = reg.emplace<Entity_Info>(entity);
            strcpy(entity_info.name, entity_name);
            entity_info.id = entity;

            Binary_Archive entity_archive(entry);

            if (!entity_archive.is_valid_id("ncomponents")) return;

            size_t ncomponents = entity_archive.read<size_t>("ncomponents");

            for (size_t i = 0; i < ncomponents; i++) {
                str_t<sizeof("component_") + 1> id = "";
                sprintf(id, "component_%llu", i + 1);

                if (entity_archive.is_valid_id(id)) {
                    str_ptr_t comp_path = entity_archive.read<path_str_t>(id);

                    path_str_t comp_name = "";
                    Path::extension_of(comp_path, comp_name);
                    void* comp = NULL;
                    uintptr_t comp_id = 0;
                    Module* comp_mod = NULL;
                    for (auto* mod : g_modules) if (mod->is_loaded && mod->get_component_id(comp_name)) {
                        comp_id = mod->get_component_id(comp_name);
                        comp = mod->create_component(comp_id, reg, entity);
                        comp_mod = mod;
                        break;
                    } 

                    if (!comp || !comp_id) continue;

                    Binary_Archive comp_archive(comp_path);

                    comp_archive.iterate([&comp_id, comp_mod, &comp, &comp_archive](str_ptr_t id) {
                        const auto& comp_info = comp_mod->get_component_info(comp_id);

                        for (auto& prop : comp_info->properties) {
                            if (strcmp(prop.name.c_str(), id) == 0) {
                                size_t actual_size = 0;
                                auto data = comp_archive.read(id, &actual_size);
                                if (actual_size > prop.size) actual_size = prop.size;
                                memcpy((byte*)comp + prop.offset, data, actual_size);
                            } 
                        }

                        return true;
                    });
                }
            }
        }
    }, false);
}

s32 filter(u32 code, _EXCEPTION_POINTERS *ep, str_ptr_t mod_name, str_ptr_t mod_fn) {
    (void)ep;
    str_ptr_t descr = "";
    switch (code) {
        case EXCEPTION_ACCESS_VIOLATION:         descr = "Access violation (Dereference nullptr? Touch garbage memory?)"; break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:    descr = "Datatype missalignment"; break;
        case EXCEPTION_BREAKPOINT:               descr = "Breakpoint"; break;
        case EXCEPTION_SINGLE_STEP:              descr = "Single step"; break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    descr = "Array bounds exceeded"; break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:     descr = "Float denormal operand"; break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:       descr = "Float division by zero"; break;
        case EXCEPTION_FLT_INEXACT_RESULT:       descr = "Float inexact result"; break;
        case EXCEPTION_FLT_INVALID_OPERATION:    descr = "Float invalid operation"; break;
        case EXCEPTION_FLT_OVERFLOW:             descr = "Float overflow"; break;
        case EXCEPTION_FLT_STACK_CHECK:          descr = "Float stack check fail"; break;
        case EXCEPTION_FLT_UNDERFLOW:            descr = "Float underflow"; break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:       descr = "Integer division by zero"; break;
        case EXCEPTION_INT_OVERFLOW:             descr = "Integer overflow"; break;
        case EXCEPTION_PRIV_INSTRUCTION:         descr = "Priv instruction"; break;
        case EXCEPTION_IN_PAGE_ERROR:            descr = "In page error"; break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: descr = "Noncontinuable expection"; break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:      descr = "Illegal instruction"; break;
        case EXCEPTION_STACK_OVERFLOW:           descr = "Stack overflow (Infinite recursion? Large function argument/stack variable?)"; break;
        case EXCEPTION_INVALID_DISPOSITION:      descr = "Invalid disposition"; break;
        case EXCEPTION_GUARD_PAGE:               descr = "Guard page"; break;
        case EXCEPTION_INVALID_HANDLE:           descr = "Invalid handle"; break;
        default:                                 descr = "N/A"; break;
    }

    log_error("System level exception thrown\nException: {} ({})\nException address: {}\nIn module '{}'\nIn function '{}'",
        descr, code, (uintptr_t)ep->ExceptionRecord->ExceptionAddress, mod_name, mod_fn);
    return EXCEPTION_EXECUTE_HANDLER;
}

template <typename ...T>
void __ignore(T && ...)
{ }

#define invoke_mod_function(mod, fn, ret_t, ...) if (mod->fn) _invoke_mod_function<ret_t>(mod->fn, mod->str_id, #fn, __VA_ARGS__)
template <typename ret_t, typename fn_t, typename ...arg_t>
ret_t _invoke_mod_function(fn_t fn, str_ptr_t mod_name, str_ptr_t fn_name, arg_t&... args) {
    __try {
        return fn(args...);
    } __except(filter(GetExceptionCode(), GetExceptionInformation(), mod_name, fn_name)) {
        if constexpr (std::is_same<void, ret_t>()) {
            return;
        } else {
            return 0;
        }
    }
}

void save_user_settings(str_ptr_t dir) {
    path_str_t user_file = "";
    sprintf(user_file, "%s/window_states", dir);
    Binary_Archive archive(user_file);
    for (auto gui_wnd : gui_windows) {
        archive.write<Gui_Window>(gui_wnd->name, *gui_wnd);
    }
    archive.flush();
}

void load_user_settings(str_ptr_t dir) {
    path_str_t user_file = "";
    sprintf(user_file, "%s/window_states", dir);
    Binary_Archive archive(user_file);
    for (auto gui_wnd : gui_windows) {
        if (!archive.is_valid_id(gui_wnd->name)) continue;
        *gui_wnd = archive.read<Gui_Window>(gui_wnd->name);

        if (gui_wnd->focused) {
            ImGui::SetWindowFocus(gui_wnd->name);
        }
    }
    archive.flush();
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

    sprintf(g_user_dir, "%s/../../.user", get_executable_directory());

    path_str_t project_dir = "";
    sprintf(project_dir, "%s/../../test_project", get_executable_directory());
    Path::to_absolute(project_dir, project_dir);

    path_str_t ecs_path = "";
    sprintf(ecs_path, "%s/ecs", project_dir);

    path_str_t style_path = "";
    sprintf(style_path, "%s/style", g_user_dir);

    path_str_t essential_dir = "";
    sprintf(essential_dir, "%s/../../essential", get_executable_directory());

    

    log_trace("ecs_path: {}", ecs_path);

    //Binary_Archive archive("test_archive");
    //int a = 1, b = 2, c = INT_MAX;
    /*str_t<128> str = "U mom";
    archive.write("a", &a, sizeof(int));
    archive.write("b", &b, sizeof(int));
    archive.write("c", &c, sizeof(int));
    archive.write("str", str, strlen(str) + 1);*/

    /*log_trace("a:   {}\n b:   {}\n c:   {}\nstr: {}", 
               archive.read<int>("a"), archive.read<int>("b"),
               archive.read<int>("c"), archive.read<str_t<6>>("str"));*/


    register_gui_window(&scene_inspector);
    register_gui_window(&entity_inspector);
    register_gui_window(&module_manager);
    register_gui_window(&log_window);
    register_gui_window(&style_editor);

    add_component_popup.is_modal = true;
    strcpy(add_component_popup.str_id, "Add Component");
    add_component_popup.fn = [](){
        for (auto* mod : g_modules)  {
            if (!mod->is_loaded) continue;
            const auto& ids = mod->get_component_ids();
            if (ids.size() > 0 && ImGui::CollapsingHeader(mod->str_id)) {
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

    g_thread_server.wait_for_thread(g_graphics_thread);

    ImGui::LoadStyleFromDisk(style_path);

    register_module("test_module");
    register_module("ecs_2d_renderer");
    register_module("asset_manager");
    register_module("2d_physics");
    register_module("2d_particles_simulator");

    for (auto* mod : g_modules) {
        log_trace("Loading module '{}'", mod->str_id);
        if (mod->load()) {
            log_info("Successfully loaded module '{}'", mod->str_id);
        } else {
            log_error("Failed loading module '{}'", mod->str_id);
        }

        invoke_mod_function(mod, on_load, void, g_graphics);
        invoke_mod_function(mod, init, void);
    }

    if (Path::exists(ecs_path)) {
        from_file(g_reg, ecs_path);
    }

    path_str_t font_path = "";
    sprintf(font_path, "%s/input_mono.ttf", essential_dir);
    ImFontConfig cfg;
    cfg.OversampleH = 8;
    cfg.OversampleV = 8;
    cfg.RasterizerMultiply = 1.f;
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 15, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 12, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 13, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 12, &cfg);
    ImGui::GetIO().Fonts->AddFontDefault();

    sprintf(font_path, "%s/louis_george.ttf", essential_dir);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 12, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 13, &cfg);
    ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 14, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 15, &cfg);

    /*sprintf(font_path, "%s/comfortaa_regular.ttf", essential_dir);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 12, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 13, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 14, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 15, &cfg);

    sprintf(font_path, "%s/jack_input.ttf", essential_dir);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 12, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 13, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 14, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 15, &cfg);

    sprintf(font_path, "%s/keep_calm.ttf", essential_dir);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 12, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 13, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 14, &cfg);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 15, &cfg);*/
    
    ImGui::GetIO().Fonts->Build();

    load_icon(ICON_TYPE_STOP,    essential_dir, "stop_icon.png");
    load_icon(ICON_TYPE_PLAY,    essential_dir, "play_icon.png");
    load_icon(ICON_TYPE_OPTIONS, essential_dir, "options_icon.png");
    load_icon(ICON_TYPE_TEXTURE, essential_dir, "texture_icon.png");
    load_icon(ICON_TYPE_FOLDER,  essential_dir, "vendor/folder_icon.png");
    load_icon(ICON_TYPE_FILE,    essential_dir, "file_icon.png");

    path_str_t ini_path = "";
    sprintf(ini_path, "%s/imgui.ini", g_user_dir);
    ImGui::GetIO().IniFilename = ini_path;

    for (auto* mod : g_modules) invoke_mod_function(mod, load_from_disk, void, project_dir);
    load_user_settings(g_user_dir);

    

    while (g_running) {
        
        auto* windows = g_graphics->get_windows_context();
        auto* wnd = windows->main_window_handle;

        g_graphics->update_imgui();

        g_thread_server.wait_for_thread(g_graphics_thread);
        ImGui::DockSpaceOverViewport();

        // Clear the color buffer
        g_graphics->clear(G_COLOR_BUFFER_BIT);

        float delta = (f32)windows->window_info[wnd].delta_time;

        
        if (g_is_playing) {
            for (auto* mod : g_modules) invoke_mod_function(mod, on_update, void, delta);
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

        for (auto* mod : g_modules) invoke_mod_function(mod, on_render, void, g_graphics);

        ImGui::UseGraphicsContext(g_graphics);
        for (auto* mod : g_modules) {
            mod->set_imgui_context(ImGui::GetCurrentContext());
            invoke_mod_function(mod, on_gui, void, g_graphics);
        }
        
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
        bool toggle_play = ImGui::IconButton((g_is_playing ? ICON_TYPE_STOP : ICON_TYPE_PLAY), { 32, 32 }, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        bar_height = ImGui::GetItemRectSize().y * 0.4f;
        if (toggle_play) {
            if (!g_is_playing) {
                to_file(g_reg, ecs_path);
                for (auto mod : g_modules) invoke_mod_function(mod, save_to_disk, void, project_dir);
                ImGui::SaveStyleToDisk(style_path);
                g_thread_server.wait_for_thread(g_graphics_thread);
                want_invoke_on_play_begin = true;
            } else {
                g_reg.clear();
                deselect_all_entities();
                from_file(g_reg, ecs_path);
                for (auto mod : g_modules) invoke_mod_function(mod, load_from_disk, void, project_dir);
                load_user_settings(g_user_dir);
                ImGui::LoadStyleFromDisk(style_path);
                g_thread_server.wait_for_thread(g_graphics_thread);
                want_invoke_on_play_stop = true;
            }

            g_is_playing = !g_is_playing;
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

            if (ImGui::MenuItem("Remove", "del", false, g_selected_entities.size() > 0) || ImGui::IsKeyPressed(AP_KEY_DELETE)) {
                for (auto entity : g_selected_entities) {
                    g_reg.destroy(entity);
                }
                deselect_all_entities();
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

                                mod->set_imgui_context(ImGui::GetCurrentContext());
                                if (component_opened) {
                                    if (info->has_custom_gui && info->properties.size() > 0) {
                                        info->properties[0].on_gui(comp);
                                    } else {
                                        for (const auto& prop : info->properties) {
                                            prop.on_gui((byte*)comp + prop.offset);
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

        ImGui::DoGuiWindow(&style_editor, []() {
            ImGui::ShowStyleEditor();
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

        if (want_invoke_on_play_begin) {
            save_user_settings(g_user_dir);
            for (auto* mod : g_modules) invoke_mod_function(mod, on_play_begin, void);
        }
        if (want_invoke_on_play_stop) {
            for (auto* mod : g_modules) invoke_mod_function(mod, on_play_stop, void);
            load_user_settings(g_user_dir);
        }

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