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

void quit() {
    g_running = false;
}

entt::registry& get_entity_registry() {
    return g_reg;
}

Thread_Server& get_thread_server() {
    return g_thread_server;
}

thread_id_t get_graphics_thread() {
    return g_graphics_thread;
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

    path_str_t mod_path = "";
    path_str_t mod_path_new = "";
    sprintf(mod_path, "%s/../runtime/test_module_used.dll", get_executable_directory());
    sprintf(mod_path_new, "%s/../runtime/test_module.dll", get_executable_directory());
        
    g_graphics_thread = g_thread_server.make_thread();

    g_graphics = new Graphics<Default_Api>();
    g_graphics->debug_callback = graphics_debug_callback;
    g_graphics->init(true, &g_thread_server, g_graphics_thread);

    typedef void (__cdecl *on_load_t)(Graphics_Context*); 
    typedef void (__cdecl *on_unload_t)(Graphics_Context*); 
    typedef void (__cdecl *on_update_t)(float); 
    typedef void (__cdecl *on_render_t)(Graphics_Context*); 
    typedef void (__cdecl *on_gui_t)(Graphics_Context*, ImGuiContext*); 

    typedef void (__cdecl *init_t)(); 
    typedef Component_Info* (__cdecl *get_component_info_t)(uintptr_t); 
    typedef const Hash_Set<uintptr_t>& (__cdecl *get_component_ids_t)(); 
    typedef void* (__cdecl *create_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef void* (__cdecl *get_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef void (__cdecl *remove_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef uintptr_t (__cdecl *get_component_id_t)(const std::string&); 

    on_load_t on_load = NULL;
    on_unload_t on_unload = NULL;
    on_update_t on_update = NULL;
    on_render_t on_render = NULL;
    on_gui_t on_gui = NULL;

    init_t init = NULL;
    get_component_info_t get_component_info = NULL;
    get_component_ids_t get_component_ids = NULL;
    create_component_t create_component = NULL;
    get_component_t get_component = NULL;
    remove_component_t remove_component = NULL;
    get_component_id_t get_component_id = NULL;

    auto err = Path::copy(mod_path_new, mod_path);
    ap_assert(err.value() == 0, "Copy fail: {}", err.message());
    
    
    #define load_fn(n) n = (n##_t)os::load_module_function(mod, #n);\
                        if (!n) { log_error("Failed loading " #n); n = NULL; }

    void* mod = NULL;

    auto load_modules = [&]() {
        mod = os::load_module(mod_path);
        if (mod) {
            load_fn(on_load);
            load_fn(on_unload);
            load_fn(on_update);
            load_fn(on_render);
            load_fn(on_gui);

            load_fn(init);
            load_fn(get_component_info);
            load_fn(get_component_ids);
            load_fn(create_component);
            load_fn(get_component);
            load_fn(remove_component);
            load_fn(get_component_id);

            log_info("Loaded test_module");

            if (on_load) on_load(g_graphics);
            if (init) init();

        } else {
            log_error("Failed loading test_module");
        }
    };

    load_modules();

    while (g_running) {
        
        auto* windows = g_graphics->get_windows_context();
        auto* wnd = windows->main_window_handle;

        g_graphics->update_imgui();

        // Clear the color buffer
        g_graphics->clear(G_COLOR_BUFFER_BIT);

        float delta = (f32)windows->window_info[wnd].delta_time;

        if (on_update) on_update(delta);

        if (windows->is_key_down(wnd, AP_KEY_R)) {
            if (mod) {
                if (on_unload) on_unload(g_graphics);
                os::free_module(mod);
            }

            err = Path::remove(mod_path);
            ap_assert(err.value() == 0, "Remove fail: {}", err.message());

            err = Path::copy(mod_path_new, mod_path);
            ap_assert(err.value() == 0, "Copy fail: {}", err.message());

            load_modules();
        } 

        if (on_render) on_render(g_graphics);

        if (on_gui) {
            ImGui::UseGraphicsContext(g_graphics);
            g_thread_server.queue_task(g_graphics_thread, [&]() {
                on_gui(g_graphics, ImGui::GetCurrentContext());    

                ImGui::Begin("Scene Inspector", NULL, ImGuiWindowFlags_MenuBar);

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

                static entt::entity selected_entity = entt::null;

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

                ImGui::End();

                ImGui::Begin("Entity Inspector");
                if (selected_entity != entt::null) {
                    const auto& ids = get_component_ids();
                    for (auto id : ids) {
                        if (void* comp = get_component(id, g_reg, selected_entity)) {
                            const auto& info = get_component_info(id);
                            bool component_opened = ImGui::TreeNodeEx(info->name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);

                            if (component_opened) {
                                for (const auto& prop : info->properties) {
                                    prop.on_gui((byte*)comp + prop.offset, ImGui::GetCurrentContext());
                                }

                                ImGui::TreePop();
                            }
                        }
                    }

                    if (ImGui::BeginMenu("Add Component")) {

                        for (auto id : ids) {
                            const auto& info = get_component_info(id);

                            if (ImGui::MenuItem(info->name.c_str())) {
                                create_component(id, g_reg, selected_entity);
                            }
                        }

                        ImGui::EndMenu();
                    }
                } else {
                    ImGui::Text("No entity selected");
                }

                ImGui::End();
            });
        }

        g_graphics->render_imgui();

        // Swap the buffers in swap chain
        windows->swap_buffers(wnd);
        
        update_dependencies();
    }

    shutdown_dependencies();

    return 0;
}