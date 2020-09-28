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

Gui_Window entity_inspector = { true, "Entity Inspector" };
Gui_Window scene_inspector = { true, "Scene Inspector" };

void quit() {
    g_running = false;
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

int AP_API start(int argc, char** argv) {
    (void)argc;
    ___set_executable_path(argv[0]);

    init_logger();
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

    register_gui_window(&scene_inspector);
    register_gui_window(&entity_inspector);
        
    g_graphics_thread = g_thread_server.make_thread();

    g_graphics = new Graphics<Default_Api>();
    g_graphics->debug_callback = graphics_debug_callback;
    g_graphics->init(true, &g_thread_server, g_graphics_thread);

    register_module("test_module");
    register_module("ecs_2d_renderer");
    register_module("asset_manager");

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

    while (g_running) {
        
        auto* windows = g_graphics->get_windows_context();
        auto* wnd = windows->main_window_handle;

        g_graphics->update_imgui();

        // Clear the color buffer
        g_graphics->clear(G_COLOR_BUFFER_BIT);

        float delta = (f32)windows->window_info[wnd].delta_time;

        
        for (auto* mod : g_modules) if (mod->on_update) mod->on_update(delta);
        

        if (windows->is_key_down(wnd, AP_KEY_R)) {
            for (auto* mod : g_modules) {
                if (mod->on_unload) mod->on_unload(g_graphics);
                mod->reload();
                if (mod->on_load) mod->on_load(g_graphics);
                if (mod->init) mod->init();
            }
        } 

        for (auto* mod : g_modules) if (mod->on_render) mod->on_render(g_graphics);

        ImGui::UseGraphicsContext(g_graphics);
        for (auto* mod : g_modules) if (mod->on_gui) mod->on_gui(g_graphics, ImGui::GetCurrentContext());    
        
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("Windows")) {
            for (auto* gui_wnd : gui_windows) {
                ImGui::MenuItem(gui_wnd->name, NULL, &gui_wnd->open);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
        static entt::entity selected_entity = entt::null;

        ImGui::DoGuiWindow(&scene_inspector, [&]() {
            ImGui::BeginMenuBar();
            if (ImGui::BeginMenu("Create")) {

                static str_t<128> buf;
                ImGui::InputText("Name", buf, sizeof(buf));

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

            g_reg.view<Entity_Info>().each([&](entt::entity entity, Entity_Info& entity_info) {
                char label[256] = "";
                sprintf(label, "%s##%i", entity_info.name, entity_info.id);

                ImGuiTreeNodeFlags flags = 0;
                flags |= ImGuiTreeNodeFlags_OpenOnArrow;
                if (entity == selected_entity) flags |= ImGuiTreeNodeFlags_Selected;
                bool entity_opened = ImGui::TreeNodeEx(label, flags);
                if (ImGui::IsItemClicked()) {
                    selected_entity = entity;
                }
                if (entity_opened) {
                    ImGui::TreePop();
                }
            });
        }, ImGuiWindowFlags_MenuBar);


        ImGui::DoGuiWindow(&entity_inspector, [&]() {
            if (selected_entity != entt::null) {
                for (auto* mod : g_modules) {
                    const auto& ids = mod->get_component_ids();
                    for (auto id : ids) {
                        if (void* comp = mod->get_component(id, g_reg, selected_entity)) {
                            const auto& info = mod->get_component_info(id);
                            bool component_opened = ImGui::TreeNodeEx(info->name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);

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
                        }
                    }
                }

                for (auto* mod : g_modules)  {
                    if (ImGui::BeginMenu("Add Component")) {
                        if (ImGui::CollapsingHeader(mod->str_id)) {
                            const auto& ids = mod->get_component_ids();
                            for (auto id : ids) {
                                const auto& info = mod->get_component_info(id);

                                if (ImGui::Selectable(info->name.c_str())) {
                                    mod->create_component(id, g_reg, selected_entity);
                                }
                            }
                        }
                        ImGui::EndMenu();
                    }
                }
            } else {
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

        // Swap the buffers in swap chain
        windows->swap_buffers(wnd);
        
        update_dependencies();

        
    }

    shutdown_dependencies();

    return 0;
}