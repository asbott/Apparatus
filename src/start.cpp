#include "pch.h"

#include <iostream>
#include <ostream>

#include "dependencies.h"

#include "graphics/graphics_api.h"
#include "graphics/opengl45.h"
#include "graphics/directx11.h"

#include "image_import.h"

#include "os.h"
#include "..\include\start.h"

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

void quit() {
    g_running = false;
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

    on_load_t on_load = NULL;
    on_unload_t on_unload = NULL;
    on_update_t on_update = NULL;
    on_render_t on_render = NULL;
    on_gui_t on_gui = NULL;

    auto err = Path::copy(mod_path_new, mod_path);
    ap_assert(err.value() == 0, "Copy fail: {}", err.message());
    auto mod = os::load_module(mod_path);
    if (mod) {

        on_load = (on_load_t)os::load_module_function(mod, "on_load");
        on_unload = (on_unload_t)os::load_module_function(mod, "on_unload");
        on_update = (on_update_t)os::load_module_function(mod, "on_update");
        on_render = (on_render_t)os::load_module_function(mod, "on_render");
        on_gui = (on_gui_t)os::load_module_function(mod, "on_gui");

        if (!on_load) {
            log_error("Failed loading on_load()");
        } else {
            on_load(g_graphics);
        }
        if (!on_update) {
            log_error("Failed loading on_update()");
        }
        if (!on_render) {
            log_error("Failed loading on_render()");
        }
        if (!on_gui) {
            log_error("Failed loading on_gui()");
        }
    } else {
        log_error("Failed loading test_module");
    }

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

            mod = os::load_module(mod_path);
            if (mod) {

                on_load = (on_load_t)os::load_module_function(mod, "on_load");
                on_unload = (on_unload_t)os::load_module_function(mod, "on_unload");
                on_update = (on_update_t)os::load_module_function(mod, "on_update");
                on_render = (on_render_t)os::load_module_function(mod, "on_render");
                on_gui = (on_gui_t)os::load_module_function(mod, "on_gui");

                if (!on_load) {
                    log_error("Failed loading on_load()");
                }
                if (!on_unload) {
                    log_error("Failed loading on_unload()");
                }
                if (!on_update) {
                    log_error("Failed loading on_update()");
                }
                if (!on_render) {
                    log_error("Failed loading on_render()");
                }
                if (!on_gui) {
                    log_error("Failed loading on_gui()");
                }

                log_info("Loaded test module");

                if (on_load) on_load(g_graphics);

                //continue;
            } else {
                on_load = NULL;
                on_unload = NULL;
                on_update = NULL;
                on_render = NULL;
                on_gui = NULL;
                log_error("Failed loading test_module");
            }
        } 

        if (on_render) on_render(g_graphics);

        if (on_gui) {
            ImGui::UseGraphicsContext(g_graphics);
            g_thread_server.queue_task(g_graphics_thread, [&]() {
                on_gui(g_graphics, ImGui::GetCurrentContext());    
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