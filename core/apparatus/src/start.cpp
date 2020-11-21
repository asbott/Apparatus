#include "pch.h"

#include <iostream>
#include <ostream>

#include <entt/meta/meta.hpp>
#include <entt/meta/ctx.hpp>
#include <entt/meta/resolve.hpp>

#include "start.h"

#include "dependencies.h"

#include "graphics/graphics_api.h"
#include "graphics/opengl45.h"
#include "graphics/directx11.h"

#include "image_import.h"

#include "os.h"

u64& uid_counter() {
    static u64 n = 0;
    return n;
}

u64 generate_unique_id() {
    return uid_counter()++;
}

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

struct Component_Reference {
    void* data = NULL;
    uintptr_t type = 0;
    Module* mod = NULL;
};

struct Entity_Preset {
    Entity_Preset(str_ptr_t name, entity_preset_fn_t fn) : name(name), fn(fn) {}
    str_ptr_t name;
    entity_preset_fn_t fn;
};

struct Window_Category_Node {

    Window_Category_Node(str_ptr_t _name) {
        strcpy(name, _name);
    }

    void assure_child(str_ptr_t _name) {
        if (used_names.count(_name) == 0) {
            child_nodes.emplace_back(_name);
            used_names.emplace(_name);
        }
    }

    void add_window(Gui_Window* wnd) {
        windows.emplace(wnd);
    }

    bool iterate(std::function<bool(Gui_Window*)> fn) {
        for (auto* wnd : windows) {
            if (!fn(wnd)) return false;
        }
        for (auto& c : child_nodes) {
            if (!c.iterate(fn)) return false;
        }
        return true;
    }

    Hash_Set<Gui_Window*> windows;
    Dynamic_Array<Window_Category_Node> child_nodes;
    Hash_Set<Dynamic_String> used_names;

    name_str_t name = "Unnamed";
};

Component_Reference component_clipboard;

Graphics_Context* g_graphics;
bool g_running = true;

Thread_Server g_thread_server;
thread_id_t g_graphics_thread;
entt::registry g_reg;

Dynamic_Array<Module*> g_modules;

Window_Category_Node* g_windows = NULL;
Hash_Set<Gui_Popup*> gui_popups;

Static_Array<graphics_id_t, ICON_TYPE_COUNT> icons;

Gui_Window entity_inspector = { true, "Entity Inspector", "Scene" };
Gui_Window scene_inspector = { true, "Scene Inspector", "Scene" };
Gui_Window module_manager = { false, "Modules Manager" };
Gui_Window log_window = { false, "Log" };
Gui_Window style_editor = { false, "Style", "Settings" };

Gui_Popup add_component_popup;
Gui_Popup manage_component_popup;
Gui_Popup file_browser_popup;

Log_Context log_context;

Game_Input game_input_state;

Hash_Set<entt::entity> g_selected_entities;

bool g_is_playing = false;

path_str_t g_user_dir = "";
path_str_t g_style_path = "";

path_str_t g_ecs_dir = "";
path_str_t g_temp_dir = "";
path_str_t g_temp_ecs_dir = "";
path_str_t g_project_dir = "";

path_str_t g_browse_dir = "";
File_Browser_Mode g_browse_mode;

Dynamic_Array<std::function<void()>> g_deferred_functions;

Dynamic_Array<Entity_Preset> g_entity_presets;

Dynamic_Array<entt::entity> g_entities_to_remove;

namespace ImGui {
	void Icon(Icon_Type icon, mz::ivec2 size) {
        auto native = g_graphics->get_native_texture_handle(icons[icon]);
        ImGui::Image(native, size);
    }
	bool IconButton(Icon_Type icon, mz::ivec2 size, const mz::color& bgr_color, bool border) {
        auto native = g_graphics->get_native_texture_handle(icons[icon]);
        if (bgr_color != mz::color(0)) ImGui::PushStyleColor(ImGuiCol_Button, bgr_color);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, (f32)border);
        bool pressed = ImGui::ImageButton(native, size, {0,0}, {1,1}, -1);
        ImGui::PopStyleVar();
        if (bgr_color != mz::color(0)) ImGui::PopStyleColor();
        return pressed;
    }

    bool InputEntity(str_ptr_t label, entt::entity* entity) {
        static char na[] = "<null>";
		if (g_reg.valid(*entity)) ImGui::RInputText(label, g_reg.get<Entity_Info>(*entity).name, sizeof(entity_name_t), ImGuiInputTextFlags_ReadOnly);
		else ImGui::RInputText(label, na, sizeof(na), ImGuiInputTextFlags_ReadOnly);

        if (ImGui::BeginDragDropTarget()) {
			auto* p = ImGui::AcceptDragDropPayload("entity");
			if (p) {
				auto payload = (Gui_Payload*)p->Data;
				auto new_entity = (entt::entity)(uintptr_t)payload->value;

                if (*entity != new_entity) {
                    *entity = new_entity;
                    return true;
                }
			}
			ImGui::EndDragDropTarget();
		}

        if (g_reg.valid(*entity)) {
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				*entity = entt::null;
                return true;
			}
		}

        return false;
    }
}

void quit() {
    g_running = false;
}

Graphics_Context* get_graphics() {
    return g_graphics;
}

Game_Input* game_input() {
    return &game_input_state;
}

entt::registry& get_entity_registry() {
    return g_reg;
}

Module* get_module(str_ptr_t str_id) {
    for (auto* mod : g_modules) {
        if (strcmp(mod->str_id, str_id) == 0) return mod;
    }
    return NULL;
}

void register_gui_window(Gui_Window* wnd) {
    
    
    /*Dynamic_String category = Dynamic_String(wnd->category);
    std::size_t last_dot = category.find_last_of('.');
    if (last_dot != Dynamic_String::npos) category = category.substr(last_dot);
    */

    std::stringstream ss(wnd->category);
    Dynamic_String category = "";
    Window_Category_Node* p = g_windows;
    while (std::getline(ss, category, '.')) {
        p->assure_child(category.c_str());
        for (auto& c : p->child_nodes) {
            if (strcmp(c.name, category.c_str()) == 0) {
                p = &c;
            }
        }
    }

    p->add_window(wnd);
}

void unregister_gui_window(Gui_Window* wnd) {
    std::function<bool(Window_Category_Node*, Gui_Window*)> rm;
    rm = [&rm](Window_Category_Node* n, Gui_Window* wnd) {
        for (auto it = n->windows.begin(); it != n->windows.end(); ++it) {
            if ((*it) == wnd) {
                n->windows.erase(it);
                return true;
            }
        }

        for (auto& c : n->child_nodes) {
            if (rm(&c, wnd)) return true;
        }
        return false;
    };

    rm(g_windows, wnd);
}

void register_gui_popup(Gui_Popup* pop) {
    gui_popups.emplace(pop);
}

void unregister_gui_popup(Gui_Popup* pop) {
    gui_popups.erase(pop);
}

void register_entity_preset(str_ptr_t name, const entity_preset_fn_t& preset_fn) {
    g_entity_presets.emplace_back(name, preset_fn );
}
void unregister_entity_preset(str_ptr_t name) {
    for (int i = (int)g_entity_presets.size() - 1; i >= 0; i--) {
        if (strcmp(g_entity_presets[i].name, name)) {
            g_entity_presets.erase(g_entity_presets.begin() + i);
        }
    }
}

void load_icon(Icon_Type type, str_ptr_t dir, str_ptr_t offset_path) {
    path_str_t icon_path = "";

    sprintf(icon_path, "%s/%s", dir, offset_path);

    mz::ivec3 icon_size;
    auto img = load_image_from_file(icon_path, &icon_size.x, &icon_size.y, &icon_size.z, 4);
    ap_assert(img != NULL, "Failed loading icon at {}: {}", offset_path, get_failure_reason());

    graphics_id_t icon = g_graphics->make_texture(G_BUFFER_USAGE_STATIC_WRITE);

    g_graphics->set_texture_filtering(icon, G_MIN_FILTER_LINEAR_MIPMAP_NEAREST, G_MAG_FILTER_NEAREST);

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

void register_module(str_ptr_t mod_name) {

    path_str_t mod_path = "";
    path_str_t mod_path_new = "";
    sprintf(mod_path, "%s/../runtime/%s_used." MODULE_FILE_EXTENSION, get_executable_directory(), mod_name);
    sprintf(mod_path_new, "%s/../runtime/%s." MODULE_FILE_EXTENSION, get_executable_directory(), mod_name);

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

void open_file_browser(File_Browser_Mode mode, std::function<void(str_ptr_t)> result_callback) {
    g_browse_mode = mode;
    file_browser_popup.should_open = true;
    if (g_browse_mode != File_Browser_Mode::file) strcpy(file_browser_popup.return_value, g_browse_dir);
    file_browser_popup.done_fn = [result_callback]() {
        if (result_callback) result_callback(file_browser_popup.return_value);
    };
}

bool is_playing() {
    return g_is_playing;
}

void defer(std::function<void()> fn) {
    g_deferred_functions.push_back(fn);
}

void to_file(entt::registry& reg, str_ptr_t dir_path) {

    auto res = Path::remove(dir_path);
    Path::create_directory(dir_path);
    reg.view<Entity_Info>().each([&reg, &dir_path](entt::entity entity, Entity_Info& entity_info) {
        path_str_t file_path = "";
        sprintf(file_path, "%s/%s%lu", dir_path, entity_info.name, (unsigned long)entity);
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
                path_str_t comp_rel_path = "";
                sprintf(comp_rel_path, "%s/%s%lu.%s", dir_path, entity_info.name, (unsigned long)entity, info->name.c_str());
                Path::to_relative(comp_rel_path, dir_path, comp_rel_path);

                str_t<sizeof("component_") + 1> id = "";
                sprintf(id, "component_%llu", (unsigned long long)ncomponents);
                entity_archive.write(id, comp_rel_path);

                path_str_t comp_abs_path = "";
                sprintf(comp_abs_path, "%s/%s", dir_path, comp_rel_path);
                Binary_Archive comp_archive(comp_abs_path);
                comp_archive.write("module", mod->str_id);
                for (const auto& prop : info->properties) {
                    entity_name_t prop_name = "";
                    strcpy(prop_name, prop.name.c_str());
                    byte* comp = (byte*) mod->get_component(comp_id, reg, entity);
                    if (prop.flags & PROPERTY_FLAG_ENTITY) {
                        entt::entity prop_entity = *(entt::entity*)(comp + prop.offset);
                        if (g_reg.valid(prop_entity)) {
                            comp_archive.write<entity_name_t>(prop_name, g_reg.get<Entity_Info>(prop_entity).name);
                        }
                    } else {
                        comp_archive.write(prop_name, comp + prop.offset, prop.size);
                    }
                }
                comp_archive.flush();
            }
        }
        entity_archive.write("ncomponents", ncomponents);

        entity_archive.flush();
    });
}

void from_file(entt::registry& reg, str_ptr_t dir_path) {
    struct Deferred_Entity_Property {
        Deferred_Entity_Property(entt::entity e, uintptr_t c, size_t po, str_ptr_t pe, Module* m)
            : entity(e), comp_id(c), prop_offset(po), mod(m) {
            strcpy(prop_entity_name, pe);
        }
        entt::entity entity;

        uintptr_t comp_id;
        size_t prop_offset;
        entity_name_t prop_entity_name;
        Module* mod;
    };

    // Entity properties that needs to be assigned AFTER all entites
    // are loaded.
    static Dynamic_Array<Deferred_Entity_Property> deferred_entity_properties;

    Path::iterate_directories(dir_path, [&reg, &dir_path](str_ptr_t entry) {
        if (Path::is_file(entry) && !Path::has_extension(entry)) {

            Binary_Archive entity_archive(entry);
            if (!entity_archive.is_valid_id("ncomponents") || !entity_archive.is_valid_id("name")) return;

            entt::entity entity = reg.create();
            entity_name_t entity_name = "";
            strcpy(entity_name, (char*)entity_archive.read("name"));
            auto& entity_info = reg.emplace<Entity_Info>(entity);
            strcpy(entity_info.name, entity_name);
            entity_info.id = entity;

            size_t ncomponents = entity_archive.read<size_t>("ncomponents");

            for (size_t i = 0; i < ncomponents; i++) {
                str_t<sizeof("component_") + 1> id = "";
                sprintf(id, "component_%llu", (unsigned long long)i + 1);
                if (entity_archive.is_valid_id(id)) {
                    str_ptr_t comp_path = entity_archive.read<path_str_t>(id);

                    path_str_t comp_name = "";
                    Path::extension_of(comp_path, comp_name);

                    path_str_t comp_real_path = "";
                    sprintf(comp_real_path, "%s/%s", dir_path, comp_path);
                    if (!Path::exists(comp_real_path)) continue;
                    Binary_Archive comp_archive(comp_real_path);

                    if (!comp_archive.is_valid_id("module") || comp_archive.size_of("module") != sizeof(Module::str_id)) {
                        log_warn("Component '{}' on entity '{}' could not be loaded because module name was not found in archive",
                                  comp_name, entity_name);
                        continue;
                    }

                    name_str_t mod_str_id = "";
                    memcpy(mod_str_id, comp_archive.read("module"), comp_archive.size_of("module"));

                    Module* comp_mod = get_module(mod_str_id);

                    if (!comp_mod || !comp_mod->is_loaded) {
                        log_error("Component '{}' on entity '{}' could not be loaded because no registered module with str_id '{}' was found",
                                   comp_name, entity_name, mod_str_id);
                        continue;
                    }

                    uintptr_t comp_id = 0;
                    void* comp = NULL;
                    for (auto type_id : comp_mod->get_component_ids()) {
                        auto info = comp_mod->get_component_info(type_id);
                        if (strcmp(info->name.c_str(), comp_name) == 0) {
                            comp_id = type_id;
                            comp = comp_mod->create_component(type_id, reg, entity);
                            break;
                        }
                    }

                    if (!comp_id || !comp) {
                        log_error("Could not find component with name '{}' when loading entity '{}'. Name might have been changed.", comp_name, entity_name);
                        continue;
                    }

                    comp_archive.iterate([&comp_id, comp_mod, &comp, &comp_archive, &entity](str_ptr_t id) {
                        const auto& comp_info = comp_mod->get_component_info(comp_id);

                        for (auto& prop : comp_info->properties) {
                            if (strcmp(prop.name.c_str(), id) == 0) {
                                size_t actual_size = 0;
                                auto data = comp_archive.read(id, &actual_size);
                                if (prop.flags & PROPERTY_FLAG_ENTITY) {    
                                    deferred_entity_properties.push_back({ entity, comp_id, prop.offset, (str_ptr_t)data, comp_mod });
                                } else {
                                    if (actual_size == prop.size) {
                                        memcpy((byte*)comp + prop.offset, data, actual_size);
                                    }
                                }
                            } 
                        }

                        return true;
                    });
                }
            }
        }
    }, false);

    for (auto& d : deferred_entity_properties) {
        g_reg.each([&](entt::entity entity) {
            auto& info = g_reg.get<Entity_Info>(entity);
            if (strcmp(info.name, d.prop_entity_name) == 0) {
                auto* comp = (byte*)d.mod->get_component(d.comp_id, g_reg, d.entity);
                memcpy(comp + d.prop_offset, &entity, sizeof(entt::entity));
            }
        });
    }

    deferred_entity_properties.clear();
}

#ifdef _OS_WINDOWS

s32 filter(u32 code, _EXCEPTION_POINTERS *ep, str_ptr_t mod_name, str_ptr_t mod_fn) {
    (void)ep; (void)mod_name; (void)mod_fn;
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

    log_error("System level exception thrown\nException: {} ({})\nException address: {}\nIn module '{}'\nWhen calling module function '{}'\nException thrown in function ''",
        descr, code, (uintptr_t)ep->ExceptionRecord->ExceptionAddress, mod_name, mod_fn);
    return EXCEPTION_EXECUTE_HANDLER;
}
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

#else
template <typename ret_t, typename fn_t, typename ...arg_t>
ret_t _invoke_mod_function(fn_t fn, str_ptr_t mod_name, str_ptr_t fn_name, arg_t&... args) {
    return fn(args...);
}
#endif

#define invoke_mod_function(mod, fn, ret_t) if (mod->fn) _invoke_mod_function<ret_t>(mod->fn, mod->str_id, #fn)
#define invoke_mod_function_args(mod, fn, ret_t, ...) if (mod->fn) _invoke_mod_function<ret_t>(mod->fn, mod->str_id, #fn, __VA_ARGS__)

void save_user_settings(str_ptr_t dir) {
    path_str_t user_file = "";
    sprintf(user_file, "%s/window_states", dir);
    Binary_Archive archive(user_file);
    g_windows->iterate([&](Gui_Window* gui_wnd) {
        archive.write<Gui_Window>(gui_wnd->name, *gui_wnd);
        return true;
    });
    for (auto& [level, color] : log_context.filter_colors) {
        name_str_t str = "";
        sprintf(str, "%s_color", spdlog::level::to_string_view(level).data());
        archive.write<mz::color>(str, color);
    }
    for (auto& [level, flag] : log_context.filter_flags) {
        name_str_t str = "";
        sprintf(str, "%s_flag", spdlog::level::to_string_view(level).data());
        archive.write<bool>(spdlog::level::to_string_view(level).data(), flag);
    }
    archive.flush();
}

void load_user_settings(str_ptr_t dir) {

    path_str_t user_file = "";
    sprintf(user_file, "%s/window_states", dir);
    Binary_Archive archive(user_file);
    g_windows->iterate([&](Gui_Window* gui_wnd) {
        if (!archive.is_valid_id(gui_wnd->name)) return false;
        *gui_wnd = archive.read<Gui_Window>(gui_wnd->name);

        if (gui_wnd->focused) {
            ImGui::SetWindowFocus(gui_wnd->name);
        }
        return true;
    });
    for (auto& [level, color] : log_context.filter_colors) {
        name_str_t str = "";
        sprintf(str, "%s_color", spdlog::level::to_string_view(level).data());
        if (archive.is_valid_id(str)) color = archive.read<mz::color>(str);
    }
    for (auto& [level, flag] : log_context.filter_flags) {
        name_str_t str = "";
        sprintf(str, "%s_flag", spdlog::level::to_string_view(level).data());
        if (archive.is_valid_id(str)) flag = archive.read<bool>(str);
    }
    archive.flush();
}

void do_file_browser_gui() {
    #ifdef _OS_WINDOWS
    str_t<3> current_disk = "  ";
    current_disk[0] = g_browse_dir[0];
    current_disk[1] = g_browse_dir[1];
    if (ImGui::RBeginCombo("Disk", current_disk)) {

        for (char c = 'A'; c <= 'Z'; c++) {
            str_t<3> disk = "  ";
            disk[0] = c;
            disk[1] = ':';
            str_t<4> disk_path;
            sprintf(disk_path, "%s/", disk);
            if (!Path::exists(disk_path)) continue;
            bool is_current = strcmp(disk, current_disk) == 0;
            if (ImGui::MenuItem(disk, NULL, is_current)) {
                if (!is_current) {
                    sprintf(g_browse_dir, "%s/", disk);
                }
            }
        }

        ImGui::REndCombo();
    }
    #endif // _OS_WINDOWS

    path_str_t prev_dir = "";
    Path::directory_of(g_browse_dir, prev_dir);
    
    static Dynamic_Array<Dynamic_String> dirs_ordered;
    while (Path::exists(prev_dir)) {
        dirs_ordered.push_back(prev_dir);
        str16_t root = "";
        Path::root_name(g_browse_dir, root);
        if (strcmp(root, prev_dir) == 0) break;
        Path::directory_of(prev_dir, prev_dir);
    }

    for (int i = (int)dirs_ordered.size() - 1; i >= 0; i--) {
        path_str_t name;
        Path::name_with_extension(dirs_ordered[i].c_str(), name);
        if (ImGui::Button(name)) {
            strcpy(g_browse_dir, dirs_ordered[i].c_str());
        }
        ImGui::SameLine();
    }

    dirs_ordered.clear();

    if (ImGui::Button("<")) {
        Path::directory_of(g_browse_dir, g_browse_dir);
        if (g_browse_mode != File_Browser_Mode::file) strcpy(file_browser_popup.return_value, g_browse_dir);
    }

    #ifdef _WIN32

    for (char c = 'A'; c <= 'Z'; c++) {
        if (strlen(g_browse_dir) == 2 && g_browse_dir[0] == c && g_browse_dir[1] == ':') {
            strcat(g_browse_dir, "/");
        }
    }

    #endif

    ImGui::Text("%s", g_browse_dir);

    ImGui::BeginChildFrame(1, { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y * 0.65f }, ImGuiWindowFlags_MenuBar);

    ImGui::BeginMenuBar();

    if (ImGui::BeginMenu("Create directory")) {
        path_str_t new_dir_name = "";
        auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter;
        
        bool enter = ImGui::RInputText("name", new_dir_name, sizeof(new_dir_name), flags, ImGui::Filters::AlphaNumericNoSpace);

        if (strcmp(new_dir_name, "") != 0 && (enter || ImGui::Button("Ok##createdir"))) {
            path_str_t full_dir = "";
            sprintf(full_dir, "%s/%s", g_browse_dir, new_dir_name);
            log_trace(full_dir);
            Path::create_directory(full_dir);
            strcpy(new_dir_name, "");

            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();

    Path::iterate_directories(g_browse_dir, [](str_ptr_t path) {
        if (Path::is_directory(path)){
            path_str_t dir_name = "";
            path_str_t ext = "";
            Path::extension_of(path, ext);
            if (ext[0] == '\0') Path::name_without_extension(path, dir_name);
            else			    Path::name_with_extension(path, dir_name);

            ImGui::Icon(ICON_TYPE_FOLDER, { 16, 16 });
            ImGui::SameLine();
            ImGui::Selectable(dir_name);
            if (ImGui::IsItemClicked()) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    strcpy(g_browse_dir, path);
                } else if (g_browse_mode != File_Browser_Mode::file) {
                    strcpy(file_browser_popup.return_value, path);
                }
            }
        }
    });

    static path_str_t file_name = "";

    Path::iterate_directories(g_browse_dir, [](str_ptr_t path) {
        if (Path::is_file(path) && g_browse_mode != File_Browser_Mode::directory) {
            path_str_t file_name = "";
            path_str_t ext = "";
            Path::extension_of(path, ext);
            if (ext[0] == '\0') Path::name_without_extension(path, file_name);
            else			    Path::name_with_extension(path, file_name);

            ImGui::Icon(ICON_TYPE_FILE, { 16, 16 });
            ImGui::SameLine();
            if (ImGui::Selectable(file_name, strcmp(path, file_browser_popup.return_value) == 0)) {
                strcpy(file_browser_popup.return_value, path);
                Path::name_with_extension(path, file_name);
            }
            if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0)) {
                if (file_browser_popup.done_fn) {
                    file_browser_popup.done_fn();
                    file_browser_popup.done_fn = NULL;
                }
                ImGui::CloseCurrentPopup();
            }
        }
    });

    ImGui::EndChildFrame();
    
    if (ImGui::InputText("File name", file_name, sizeof(file_name))) {
        sprintf(file_browser_popup.return_value, "%s/%s", g_browse_dir, file_name);
    }

    ImGui::Text("Selected: %s", file_browser_popup.return_value);

    if (ImGui::Button("Ok")) {
        if (file_browser_popup.done_fn) {
            file_browser_popup.done_fn();
            file_browser_popup.done_fn = NULL;
        }
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
        file_browser_popup.done_fn = NULL;
    }
}

bool load_module(Module* mod) {
    if (!mod->load()) return false;
    ap_assert(mod->init, "Init function missing in module '{}'", mod->str_id);
    mod->init(mod);

    invoke_mod_function(mod, on_load, void);
    for (auto& deferred_fn : g_deferred_functions) deferred_fn();
    g_deferred_functions.clear();

    return true;
}

bool unload_module(Module* mod) {
    invoke_mod_function(mod, on_unload, void);
    deselect_all_entities();
    if (mod->is_loaded) {
        for (auto comp_id : mod->get_component_ids()) {
            g_reg.each([mod, comp_id](entt::entity entity) {
                if (mod->get_component(comp_id, g_reg, entity)) {
                    mod->remove_component(comp_id, g_reg, entity);
                }
            });
        }
        //mod->deinit();
    }
    if (!mod->unload()) return false;
    return true;
}

bool reload_module(Module* mod) {
    return unload_module(mod) && load_module(mod);
}

void save_project() {
    if (is_playing()) return;
    for (auto mod : g_modules) invoke_mod_function_args(mod, save_to_disk, void, g_project_dir);
    for (auto& deferred_fn : g_deferred_functions) deferred_fn();
    g_deferred_functions.clear();

    to_file(g_reg, g_ecs_dir);
    save_user_settings(g_user_dir);
    ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    ImGui::SaveStyleToDisk(g_style_path);
}

void load_project(str_ptr_t new_dir) {
    if (is_playing()) return;
    strcpy(g_project_dir, new_dir);
    sprintf(g_ecs_dir, "%s/ecs", new_dir);
    sprintf(g_temp_dir, "%s/.temp", new_dir);
    sprintf(g_temp_ecs_dir, "%s/ecs", g_temp_dir);

    g_reg.clear();
    deselect_all_entities();
    for (auto mod : g_modules) if (mod->is_loaded) reload_module(mod); else load_module(mod);
    from_file(g_reg, g_ecs_dir);

    for (auto mod : g_modules) invoke_mod_function_args(mod, load_from_disk, void, g_project_dir);
    for (auto& deferred_fn : g_deferred_functions) deferred_fn();
    g_deferred_functions.clear();

    load_user_settings(g_user_dir);
    ImGui::LoadIniSettingsFromDisk(ImGui::GetIO().IniFilename);
    ImGui::LoadStyleFromDisk(g_style_path);
}

void do_entity_management_gui() {
    if (ImGui::BeginMenu("Create")) {

        static str_t<128> buf;

        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue
                                  | ImGuiInputTextFlags_CallbackCharFilter;

        entt::entity entity = entt::null;

        bool create = false;

        ImGui::RInputText("Name", buf, sizeof(buf), flags, ImGui::Filters::AlphaNumeric);

        ImGui::Separator();

        for (const auto& entity_preset : g_entity_presets) {
            if (ImGui::MenuItem(entity_preset.name, 0, false, strlen(buf))) {
                entity = g_reg.create();
                entity_preset.fn(g_reg, entity);
                create = true;
                break;
            }
        }

        if (create) {
            g_reg.emplace<Entity_Info>(entity);
            g_reg.get<Entity_Info>(entity).id = entity;
            strcpy(g_reg.get<Entity_Info>(entity).name, buf);
            strcpy(buf, "");
        }

        if (ImGui::IsKeyReleased(AP_KEY_ENTER)) ImGui::CloseCurrentPopup();

        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Remove", "del", false, g_selected_entities.size() > 0) || ImGui::IsKeyPressed(AP_KEY_DELETE)) {
        for (auto entity : g_selected_entities) {
            g_entities_to_remove.push_back(entity);
        }
        deselect_all_entities();
    }
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

    sprintf(g_project_dir, "%s/../../test_project", get_executable_directory());
    Path::to_absolute(g_project_dir, g_project_dir);

    strcpy(g_browse_dir, g_project_dir);

    sprintf(g_ecs_dir, "%s/ecs", g_project_dir);

    sprintf(g_style_path, "%s/style", g_user_dir);

    path_str_t essential_dir = "";
    sprintf(essential_dir, "%s/../../essential", get_executable_directory());

    g_windows = new Window_Category_Node("Windows");

    register_gui_window(&scene_inspector);
    register_gui_window(&entity_inspector);
    register_gui_window(&module_manager);
    register_gui_window(&log_window);
    register_gui_window(&style_editor);

    add_component_popup.is_modal = true;
    strcpy(add_component_popup.str_id, "Add Component");
    add_component_popup.fn = [](){
        static int sort_mode = 0;
        static auto sort_mode_str = [](int sort_mode) {
            switch (sort_mode) {
                case 0: return "module";
                case 1: return "name";
                default: return "N/A";
            }
        };

        if (ImGui::RBeginCombo("Sort by", sort_mode_str(sort_mode))) {

            if (ImGui::Selectable(sort_mode_str(0))) {
                sort_mode = 0;
            }
            if (ImGui::Selectable(sort_mode_str(1))) {
                sort_mode = 1;
            }

            ImGui::REndCombo();
        }

        struct Comp_Entry {
            str_ptr_t name;
            uintptr_t id;
            Module* mod;
            Icon_Type icon;
        };
        static Dynamic_Array<Comp_Entry> sorted_names;

        for (auto* mod : g_modules)  {
            if (!mod->is_loaded) continue;
            const auto& ids = mod->get_component_ids();
            
            for (auto id : ids) {
                const auto& info = mod->get_component_info(id);
                sorted_names.push_back({ info->name.c_str(), id, mod, info->icon });
            }
        }

        static auto str_score = [](str_ptr_t str, u32 n) {
            u32 score = 0;
            for (size_t i = 0; i < (n > strlen(str) ? strlen(str) : n); i++) {
                if (isalpha(str[i])) score += (u32)'Z' - (u32)toupper(str[i]);
                else score += (u32)str[i];
            }
            return score;
        };

        if (sort_mode == 0) {
            std::sort(sorted_names.begin(), sorted_names.end(), [](const Comp_Entry& a, const Comp_Entry& b) {
                for (int i = 0; i < (strlen(a.mod->str_id) > strlen(b.mod->str_id) ? strlen(b.mod->str_id) : strlen(a.mod->str_id)); i++) {
                    if (str_score(a.mod->str_id, i) > str_score(b.mod->str_id, i)) return true;
                    if (str_score(a.mod->str_id, i) < str_score(b.mod->str_id, i)) return false;
                }
                return a.id > b.id;
            });
        } else if (sort_mode == 1) {
            std::sort(sorted_names.begin(), sorted_names.end(), [](const Comp_Entry& a, const Comp_Entry& b) {
                for (int i = 0; i < (strlen(a.name) > strlen(b.name) ? strlen(b.name) : strlen(a.name)); i++) {
                    if (str_score(a.name, i) > str_score(b.name, i)) return true;
                    if (str_score(a.name, i) < str_score(b.name, i)) return false;
                }
                return a.id > b.id;
            });
        }

        static comp_name_t search_filter = "";
        ImGui::RInputText("Filter", search_filter, sizeof(search_filter));

        for (auto entry : sorted_names) {

            bool any_match = false;
            comp_name_t search_filter_upper = "";
            strcpy(search_filter_upper, search_filter);
            for (size_t i = 0; i < strlen(search_filter); i++) search_filter_upper[i] = toupper(search_filter[i]);
            for (size_t i = 0; i < strlen(entry.name); i++) {
                comp_name_t entry_name_upper = "";
                strcpy(entry_name_upper, entry.name);
                for (size_t j = 0; j < strlen(entry.name); j++) entry_name_upper[j] = toupper(entry.name[j]);
                if (strlen(search_filter_upper) > strlen(entry_name_upper + i)) break;
                if (memcmp(entry_name_upper + i, search_filter_upper, strlen(search_filter_upper)) == 0) {
                    any_match = true;
                    break;
                }
            }
            if (!any_match) continue;

            ImGuiSelectableFlags flags = ImGuiSelectableFlags_None;
            if (g_selected_entities.size() == 1 && entry.mod->has_component(entry.id, g_reg, *g_selected_entities.begin())) {
                flags |= ImGuiSelectableFlags_Disabled;
            }
            f32 sz = ImGui::GetIO().FontDefault->FontSize;
            ImGui::Icon(entry.icon, { sz, sz });
            ImGui::SameLine();
            if (ImGui::Selectable(entry.name, false, flags)) {
                for (auto selected_entity : g_selected_entities) {
                    entry.mod->create_component(entry.id, g_reg, selected_entity);
                }
                memset(search_filter, 0, sizeof(search_filter));
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            name_str_t mod_str = "";
            sprintf(mod_str, "(%s)", entry.mod->str_id);
            ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(mod_str).x - ImGui::GetWindowContentRegionWidth() * .01f);
            ImGui::TextDisabled("%s", mod_str);

            ImGui::Spacing();
        }

        sorted_names.clear();

        if (ImGui::Button("Cancel")) {
            memset(search_filter, 0, sizeof(search_filter));
            ImGui::CloseCurrentPopup();
        }
    };

    manage_component_popup.is_modal = false;
    strcpy(manage_component_popup.str_id, "");

    strcpy(file_browser_popup.str_id, "File Browser");
    file_browser_popup.is_modal = true;
    file_browser_popup.fn = []() {

        do_file_browser_gui();
    };

    register_gui_popup(&add_component_popup);
    register_gui_popup(&manage_component_popup);
    register_gui_popup(&file_browser_popup);


        
    g_graphics_thread = g_thread_server.make_thread();

    g_graphics = new Graphics<Default_Api>();
    g_graphics->debug_callback = graphics_debug_callback;
    g_graphics->init(true, &g_thread_server, g_graphics_thread);

    g_thread_server.wait_for_thread(g_graphics_thread);

    register_module("asset_manager");
    register_module("2d_viewport");
    register_module("2d_editor");

    register_module("2d_sprite_renderer");
    register_module("2d_tilemap_renderer");
    register_module("2d_particles_simulator");

    register_module("2d_physics");

    register_module("test_module");

    ImGui::GetIO().Fonts->AddFontDefault();

    path_str_t font_dir = "";
    sprintf(font_dir, "%s/fonts", essential_dir);
    path_str_t font_path = "";

    sprintf(font_path, "%s/louis_george.ttf", font_dir);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 12);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 13);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 14);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 15);

    sprintf(font_path, "%s/gontserrat_regular.ttf", font_dir);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 13);
    ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 14);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 15);

    sprintf(font_path, "%s/gontserrat_semibold.ttf", font_dir);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 13);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 14);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path, 15);
    
    ImGui::GetIO().Fonts->Build();

    load_icon(ICON_TYPE_STOP,      essential_dir, "stop_icon.png");
    load_icon(ICON_TYPE_PLAY,      essential_dir, "play_icon.png");
    load_icon(ICON_TYPE_OPTIONS,   essential_dir, "options_icon.png");
    load_icon(ICON_TYPE_TEXTURE,   essential_dir, "texture_icon.png");
    load_icon(ICON_TYPE_FOLDER,    essential_dir, "vendor/folder_icon.png");
    load_icon(ICON_TYPE_FILE,      essential_dir, "file_icon.png");
    load_icon(ICON_TYPE_DATA,      essential_dir, "data_icon.png");

    path_str_t ini_path = "";
    sprintf(ini_path, "%s/imgui.ini", g_user_dir);
    ImGui::GetIO().IniFilename = ini_path;

    load_project(g_project_dir);

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
            for (auto* mod : g_modules) invoke_mod_function_args(mod, on_update, void, delta);
            for (auto& deferred_fn : g_deferred_functions) deferred_fn();
            g_deferred_functions.clear();
        }

        if (windows->should_close(wnd)) {
            quit();
        }

        log_context.do_gui(&log_window);

        static Dynamic_Array<Module*> to_reload;
        static Dynamic_Array<Module*> to_unload;
        static Dynamic_Array<Module*> to_load;

        ImGui::DoGuiWindow(&module_manager, []() {
            static Module* selected_module = NULL;
            
            if (ImGui::RBeginCombo("Module", selected_module ? selected_module->str_id : "<none>")) {
                for (auto mod : g_modules) {
                    if (mod && ImGui::Selectable(mod->str_id, mod == selected_module)) {
                        selected_module = mod;
                    }
                }   

                ImGui::REndCombo();
            }

            ImGui::Separator();
            if (selected_module) {
                ImGui::RCheckboxReadonly("Module", selected_module->str_id);
                ImGui::RCheckboxReadonly("Has function 'on_load'",selected_module->on_load != NULL);
                ImGui::RCheckboxReadonly("Has function 'on_unload'", selected_module->on_unload != NULL);
                ImGui::RCheckboxReadonly("Has function 'on_update'", selected_module->on_update != NULL);
                ImGui::RCheckboxReadonly("Has function 'on_render'", selected_module->on_render != NULL);
                ImGui::RCheckboxReadonly("Has function 'on_gui'", selected_module->on_gui != NULL);
                ImGui::RCheckboxReadonly("Has function 'on_play_begin'", selected_module->on_play_begin != NULL);
                ImGui::RCheckboxReadonly("Has function 'on_play_stop'", selected_module->on_play_stop != NULL);
                ImGui::RCheckboxReadonly("Has function 'save_to_disk'", selected_module->save_to_disk != NULL);
                ImGui::RCheckboxReadonly("Has function 'load_from_disk'", selected_module->load_from_disk != NULL);
                ImGui::RCheckboxReadonly("Has function 'get_function_library'", selected_module->get_function_library != NULL);

                if (selected_module->is_loaded) {
                    ImGui::Spacing();
                    ImGui::Text("Number of components: %lu", selected_module->get_component_ids().size());
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

        for (auto* mod : g_modules) invoke_mod_function(mod, on_render, void); 
        for (auto& deferred_fn : g_deferred_functions) deferred_fn();
        g_deferred_functions.clear();

        ImGui::UseGraphicsContext(g_graphics);
        for (auto* mod : g_modules) {
            if (!mod->is_loaded) continue;
            mod->set_imgui_context(ImGui::GetCurrentContext());
            invoke_mod_function(mod, on_gui, void);
        }
        for (auto& deferred_fn : g_deferred_functions) deferred_fn();
        g_deferred_functions.clear();   
        
        static f32 bar_width = (f32)windows->window_info[wnd].size.width;
        static f32 bar_height = 32.f;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { bar_width, bar_height });
        ImGui::BeginMainMenuBar();
        ImGui::PopStyleVar();
        bar_width = ImGui::GetWindowWidth();

        if (ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && ImGui::IsKeyDown(AP_KEY_S)) {
            save_project();
        }
        if (ImGui::BeginMenu("File")) {
            
            if (ImGui::MenuItem("New project...")) {
                open_file_browser(File_Browser_Mode::directory, [](str_ptr_t result) {
                    ap_assert(Path::exists(result) && Path::is_directory(result), "Invalid project directory");
                    load_project(result);

                    save_project();
                });
            }

            if (ImGui::MenuItem("Save project", "ctrl+s")) {
                save_project();
            }

            if (ImGui::MenuItem("Load project...")) {
                open_file_browser(File_Browser_Mode::directory, [](str_ptr_t result) {
                    ap_assert(Path::exists(result) && Path::is_directory(result), "Invalid project directory");
                    load_project(result);
                });
            }

            ImGui::Separator();

            ImGui::EndMenu();
        }

        std::function<void(Window_Category_Node* n)> wnd_gui;
        wnd_gui = [&](Window_Category_Node* n) {
            if (ImGui::BeginMenu(n->name)) {
                for (auto& c : n->child_nodes) {
                    wnd_gui(&c);
                }
                for (auto gui_wnd : n->windows) {
                    ImGui::MenuItem(gui_wnd->name, NULL, &gui_wnd->open);
                }
                if (n->child_nodes.size() > 0) {
                    ImGui::Separator();
                    ImGui::PushID(n);
                    if (ImGui::BeginMenu("All")) {
                        n->iterate([](Gui_Window* gui_wnd) {
                            ImGui::MenuItem(gui_wnd->name, NULL, &gui_wnd->open);
                            return true;
                        });
                        ImGui::EndMenu();
                    }
                    ImGui::PopID();
                }
                ImGui::EndMenu();
            }
        };

        wnd_gui(g_windows);

        ImGui::Separator();

        bool want_invoke_on_play_begin = false;
        bool want_invoke_on_play_stop   = false;

        ImGui::Indent(ImGui::GetWindowWidth() / 2.f - 16);
        bool toggle_play = ImGui::IconButton((g_is_playing ? ICON_TYPE_STOP : ICON_TYPE_PLAY), { 32, 32 }, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        bar_height = ImGui::GetItemRectSize().y * 0.4f;
        if (toggle_play) {
            if (!g_is_playing) {
                to_file(g_reg, g_temp_ecs_dir);
                
                g_thread_server.wait_for_thread(g_graphics_thread);
                want_invoke_on_play_begin = true;
            } else {

                g_thread_server.wait_for_thread(g_graphics_thread);
                
                static Dynamic_Array<str_ptr_t> selected_names;
                for (auto& selected_entity : get_selected_entities()) {
                    selected_names.push_back(g_reg.get<Entity_Info>(selected_entity).name);
                }
                deselect_all_entities();

                g_reg.each([](entt::entity entity) {
                    g_reg.destroy(entity);
                });
                from_file(g_reg, g_temp_ecs_dir);

                for (auto n : selected_names) {
                    g_reg.view<Entity_Info>().each([n](entt::entity entity, Entity_Info& info) {
                        if (strcmp(info.name, n) == 0) {
                            select_entity(entity);
                        }
                    });
                }
                selected_names.clear();
                want_invoke_on_play_stop = true;
            }

            g_is_playing = !g_is_playing;
        }
 
        ImGui::EndMainMenuBar();

        ImGui::DoGuiWindow(&scene_inspector, [&]() {

            ImGui::BeginMenuBar();
            do_entity_management_gui();
            ImGui::EndMenuBar();

            bool any_item_popup = false;
            g_reg.view<Entity_Info>().each([&](entt::entity entity, Entity_Info& entity_info) {
                char label[256] = "";
                sprintf(label, "%s##%i", entity_info.name, entity_info.id);

                if (ImGui::Selectable(entity_info.name, g_selected_entities.count(entity) > 0)) {
                    if (!ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && !ImGui::IsKeyDown(AP_KEY_RIGHT_CONTROL)) {
                        deselect_all_entities();
                    }
                    select_entity(entity);
                }

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
                    Gui_Payload payload;
                    payload.value = (void*)(uintptr_t)entity;
                    ImGui::SetDragDropPayload("entity", &payload, sizeof(Gui_Payload));
                    ImGui::Text(g_reg.get<Entity_Info>(entity).name);
                    ImGui::EndDragDropSource();
                }

                ImGui::PushID((int)entity);
                if (ImGui::BeginPopupContextItem("")) {
                    any_item_popup = true;
                    if (ImGui::MenuItem("Remove")) {
                        g_entities_to_remove.push_back(entity);
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            });

            if (!any_item_popup && ImGui::BeginPopupContextWindow("scene_inspector")) {
                do_entity_management_gui();
                ImGui::EndPopup();
            }

            if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && ImGui::IsWindowHovered()) {
                deselect_all_entities();
            }
        }, ImGuiWindowFlags_MenuBar);
        
        ImGui::DoGuiWindow(&entity_inspector, [&]() {
            for (auto selected_entity : g_selected_entities) {
                if (g_reg.valid(selected_entity)) {
                    auto& entity_info = g_reg.get<Entity_Info>(selected_entity);
                    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue
                                    | ImGuiInputTextFlags_CallbackCharFilter;
                    ImGui::RInputText("Name", entity_info.name, sizeof(entity_info.name), flags, ImGui::Filters::AlphaNumeric);
                    ImGui::Separator();
                    for (auto* mod : g_modules) {
                        if (!mod->is_loaded) continue;
                        const auto& ids = mod->get_component_ids();
                        for (auto id : ids) {
                            if (void* comp = mod->get_component(id, g_reg, selected_entity)) {
                                const auto& info = mod->get_component_info(id);
                                

                                f32 header_pos_y = ImGui::GetCursorPosY();

                                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow 
                                                            | ImGuiTreeNodeFlags_DefaultOpen 
                                                            | ImGuiTreeNodeFlags_OpenOnDoubleClick 
                                                            //| ImGuiTreeNodeFlags_CollapsingHeader 
                                                            | ImGuiTreeNodeFlags_NoAutoOpenOnLog
                                                            | ImGuiTreeNodeFlags_SpanFullWidth;

                                comp_name_t str_id = "";
                                sprintf(str_id, "##%s", info->name.c_str());
                                bool component_opened = ImGui::TreeNodeEx(str_id, flags);
                                ImGui::SameLine();
                                ImGui::Icon(info->icon, mz::ivec2((s32)ImGui::GetIO().FontDefault->FontSize));
                                ImGui::SameLine();
                                ImGui::Text("%s", info->name.c_str());

                                if (ImGui::IsItemClicked(1)) {
                                    manage_component_popup.should_open = true;
                                    manage_component_popup.fn = [mod, id, selected_entity, comp, info]() {
                                        if (ImGui::MenuItem("Remove")) {
                                            mod->remove_component(id, g_reg, selected_entity);
                                        }
                                        if (ImGui::MenuItem("Copy")) {
                                            component_clipboard = { comp, id, mod };
                                        }
                                        if (ImGui::MenuItem("Paste", 0, false, id == component_clipboard.type && component_clipboard.mod == mod && component_clipboard.data)) {
                                            memcpy(comp, component_clipboard.data, info->size);
                                        }
                                        
                                    };
                                }

                                ImGui::SameLine();
                                name_str_t mod_str = "";
                                sprintf(mod_str, "(%s)", mod->str_id);
                                ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(mod_str).x - ImGui::GetWindowContentRegionWidth() * .01f);
                                ImGui::TextDisabled("%s", mod_str);

                                mod->set_imgui_context(ImGui::GetCurrentContext());
                                if (component_opened) {
                                    ImGui::Separator();
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

                    

                    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

                    static f32 button_width = 100.f;

                    ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.f - button_width / 2.f);

                    if (ImGui::RButton("Add Component")) {
                        add_component_popup.should_open = true;
                    }
                    button_width = ImGui::GetItemRectSize().x;
                }
            }
            if (g_selected_entities.size() == 0) {
                ImGui::Text("No entity selected");
            }
        });

        ImGui::DoGuiWindow(&style_editor, [&]() {
            auto& style = ImGui::GetStyle();
            auto& ext_style = ImGui::GetExtensionStyle();

            ImGui::BeginMenuBar();

            if (ImGui::BeginMenu("Presets...")) {
                path_str_t presets_dir = "";
                sprintf(presets_dir, "%s/styles", essential_dir);
                Path::iterate_directories(presets_dir, [](str_ptr_t entry) {
                    if (!Path::is_file(entry)) return true;

                    path_str_t file_name = "";

                    Path::name_without_extension(entry, file_name);

                    if (ImGui::MenuItem(file_name)) {
                        ImGui::LoadStyleFromDisk(entry);
                    }

                    return true;
                });
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();

            if (ImGui::RButton("Save to file")) {
                open_file_browser(File_Browser_Mode::file, [](str_ptr_t result) {
                    ImGui::SaveStyleToDisk(result);
                });
            }

            if (ImGui::RButton("Load from file")) {
                open_file_browser(File_Browser_Mode::file, [](str_ptr_t result) {
                    ImGui::LoadStyleFromDisk(result);
                });
            }

            ImGui::Separator();

            ImGui::Text("Padding");
            ImGui::RDragFvec2("Window padding", (fvec2*)&style.WindowPadding, .05f);
            ImGui::RDragFvec2("Frame padding", (fvec2*)&style.FramePadding, .05f);
            ImGui::RDragFvec2("Touch extra padding", (fvec2*)&style.TouchExtraPadding, .05f);
            ImGui::RDragFvec2("Display window padding", (fvec2*)&style.DisplayWindowPadding, .05f);
            ImGui::RDragFvec2("Display safe area padding", (fvec2*)&style.DisplaySafeAreaPadding, .05f);
            ImGui::RDragFvec2("Item padding", (fvec2*)&style.ItemSpacing, .05f);
            ImGui::RDragFvec2("Item inner padding", (fvec2*)&style.ItemInnerSpacing, .05f);
            ImGui::RDragFloat("Indent padding", &style.IndentSpacing, .05f);
            ImGui::RDragFloat("Columns min padding", &style.ColumnsMinSpacing, .05f);
            ImGui::RSliderFloat("Right align padding", &ext_style.right_align_padding, .01f, 1.f);
            ImGui::RDragFvec2("Button min padding", (fvec2*)&ext_style.min_button_padding, .05f);

            ImGui::Spacing();
            ImGui::Text("Border toggles");
            {
                ImGui::PushID(&style.WindowBorderSize);
                bool borders = style.WindowBorderSize > 0;
                ImGui::RCheckbox("Window borders", &borders);
                style.WindowBorderSize = borders ? 1.f : 0.f;
                ImGui::PopID();
            }
            {
                ImGui::PushID(&style.ChildBorderSize);
                bool borders = style.ChildBorderSize > 0;
                ImGui::RCheckbox("Child borders", &borders);
                style.ChildBorderSize = borders ? 1.f : 0.f;
                ImGui::PopID();
            }
            {
                ImGui::PushID(&style.PopupBorderSize);
                bool borders = style.PopupBorderSize > 0;
                ImGui::RCheckbox("Popup borders", &borders);
                style.PopupBorderSize = borders ? 1.f : 0.f;
                ImGui::PopID();
            }
            {
                ImGui::PushID(&style.TabBorderSize);
                bool borders = style.TabBorderSize > 0;
                ImGui::RCheckbox("Tab borders", &borders);
                style.TabBorderSize = borders ? 1.f : 0.f;
                ImGui::PopID();
            }

            {
                ImGui::PushID(&style.FrameBorderSize);
                bool borders = style.FrameBorderSize > 0;
                ImGui::RCheckbox("Frame borders", &borders);
                style.FrameBorderSize = borders ? 1.f : 0.f;
                ImGui::PopID();
            }

            

            ImGui::Spacing();
            ImGui::Text("Size");
            ImGui::RDragFvec2("Window min size", (fvec2*)&style.WindowMinSize, .05f, .1f, 20000.f);
            ImGui::RDragFloat("Scrollbar size", &style.ScrollbarSize, .1f, .1f, 1000.f);
            ImGui::RDragFloat("Grab min size", &style.GrabMinSize, .1f, .1f, 1000.f);
            ImGui::RDragFloat("Tab min width for close button", &style.TabMinWidthForCloseButton);

            ImGui::Spacing();
            ImGui::Text("Rounding");
            ImGui::RSliderFloat("Window rounding", &style.WindowRounding, .0f, 40.f);
            ImGui::RSliderFloat("Child rounding", &style.ChildRounding, .0f, 40.f);
            ImGui::RSliderFloat("Popup rounding", &style.PopupRounding, .0f, 40.f);
            ImGui::RSliderFloat("Frame rounding", &style.FrameRounding, .0f, 40.f);
            ImGui::RSliderFloat("Scrollbar rounding", &style.ScrollbarRounding, .0f, 40.f);
            ImGui::RSliderFloat("Grab rounding", &style.GrabRounding, .0f, 40.f);
            ImGui::RSliderFloat("Tab rounding", &style.TabRounding, .0f, 40.f);

            ImGui::Spacing();
            ImGui::Text("Alignment");
            ImGui::RSliderFloat2("Window title align", (float*)&style.WindowTitleAlign, .0f, 1.f);
            ImGui::RSliderFloat2("Button text align", (float*)&style.ButtonTextAlign, .0f, 1.f);
            ImGui::RSliderFloat2("Selectable text align", (float*)&style.SelectableTextAlign, .0f, 1.f);

            auto imgui_dir_str = [](ImGuiDir dir) {
                switch (dir) {
                    case ImGuiDir_Up:    return "Top";
                    case ImGuiDir_Right: return "Right";
                    case ImGuiDir_Down:  return "Bottom";
                    case ImGuiDir_Left:  return "Left";
                }
                return "N/A";
            };

            if (ImGui::RBeginCombo("Window menu button align", imgui_dir_str(style.WindowMenuButtonPosition))) {
                if (ImGui::Selectable("None", style.WindowMenuButtonPosition == ImGuiDir_None)) {
                    style.WindowMenuButtonPosition = ImGuiDir_None;
                }
                if (ImGui::Selectable("Right", style.WindowMenuButtonPosition == ImGuiDir_Right)) {
                    style.WindowMenuButtonPosition = ImGuiDir_Right;
                }
                if (ImGui::Selectable("Left", style.WindowMenuButtonPosition == ImGuiDir_Left)) {
                    style.WindowMenuButtonPosition = ImGuiDir_Left;
                }
                ImGui::REndCombo();
            }

            if (ImGui::RBeginCombo("Color button align", imgui_dir_str(style.ColorButtonPosition))) {
                if (ImGui::Selectable("Right", style.WindowMenuButtonPosition == ImGuiDir_Right)) {
                    style.ColorButtonPosition = ImGuiDir_Right;
                }
                if (ImGui::Selectable("Left", style.WindowMenuButtonPosition == ImGuiDir_Left)) {
                    style.ColorButtonPosition = ImGuiDir_Left;
                }
                ImGui::REndCombo();
            }
            ImGui::RSliderFloat("Right align", &ext_style.right_align_padding, .01f, 1.f);

            ImGui::Spacing();
            ImGui::Text("Rendering");
            ImGui::RCheckbox("Anti-aliased lines", &style.AntiAliasedLines);
            ImGui::RCheckbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
            ImGui::RCheckbox("Anti-aliased fill", &style.AntiAliasedFill);

            ImGui::Spacing();
            ImGui::Text("Colors");
            for (int i = 0; i < ImGuiCol_COUNT; i++) {
                ImGui::RColorEdit4(ImGui::GetStyleColorName(i), (float*)&style.Colors[i]);
            } 
        }, ImGuiWindowFlags_MenuBar);

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
            for (auto& deferred_fn : g_deferred_functions) deferred_fn();
            g_deferred_functions.clear();   
        }
        if (want_invoke_on_play_stop) {
            for (auto* mod : g_modules) invoke_mod_function(mod, on_play_stop, void);
            load_user_settings(g_user_dir);
            for (auto& deferred_fn : g_deferred_functions) deferred_fn();
            g_deferred_functions.clear();   
        }

        // Swap the buffers in swap chain
        windows->swap_buffers(wnd);
        
        update_dependencies();

        if (to_reload.size() > 0) {
            to_file(g_reg, g_temp_ecs_dir);
            g_reg.clear();
        };
        for (auto* mod : to_reload) {
            reload_module(mod);
        }
        if (to_reload.size() > 0) {
            from_file(g_reg, g_temp_ecs_dir);
        }
        to_reload.clear();

        for (auto* mod : to_load) {
            load_module(mod);
        }
        to_load.clear();

        for (auto* mod : to_unload) {
            unload_module(mod);
        }
        to_unload.clear();

        for (auto to_remove : g_entities_to_remove) {
            if (is_entity_selected(to_remove)) {
                deselect_entity(to_remove);
            }
            g_reg.destroy(to_remove);
        }
        g_entities_to_remove.clear();
    }

    delete g_windows;

    shutdown_dependencies();

    log_stream.close();

    return 0;
}