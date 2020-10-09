#include "pch.h"

#ifdef AP_SUPPORT_OPENGL45

#include <glad/glad.h>
#include <gl/GLU.h>

#include <glfw/glfw3.h>

#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_glfw.h>

#include "graphics/opengl45.h"

#include "graphics/graphics_debug_macros.h"

#define make_context_current(...) //if(glfwGetCurrentContext() != (GLFWwindow*)_windows_context.main_window_handle) glfwMakeContextCurrent((GLFWwindow*)_windows_context.main_window_handle);

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei /*length*/, 
                            const char *message, 
                            const void *userParam)
{
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    char source_str  [64] = "N/A";
    char type_str    [64] = "N/A";
    char severity_str[64] = "N/A";

    graphics_enum_t g_severity = 0;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             strcpy(source_str, "API");             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   strcpy(source_str, "Window System");   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: strcpy(source_str, "Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     strcpy(source_str, "Third Party");     break;
        case GL_DEBUG_SOURCE_APPLICATION:     strcpy(source_str, "Application");     break;
        case GL_DEBUG_SOURCE_OTHER:           strcpy(source_str, "Other");           break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               strcpy(type_str, "Error");                break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: strcpy(type_str, "Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  strcpy(type_str, "Undefined Behaviour");  break; 
        case GL_DEBUG_TYPE_PORTABILITY:         strcpy(type_str, "Portability");          break;
        case GL_DEBUG_TYPE_PERFORMANCE:         strcpy(type_str, "Performance");          break;
        case GL_DEBUG_TYPE_MARKER:              strcpy(type_str, "Marker");               break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          strcpy(type_str, "Push Group");           break;
        case GL_DEBUG_TYPE_POP_GROUP:           strcpy(type_str, "Pop Group");            break;
        case GL_DEBUG_TYPE_OTHER:               strcpy(type_str, "Other");                break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         
            strcpy(severity_str, "high");        
            g_severity = G_DEBUG_MESSAGE_SEVERITY_CRITICAL;
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:       
            strcpy(severity_str, "medium");       
            g_severity = G_DEBUG_MESSAGE_SEVERITY_ERROR;
            break;
        case GL_DEBUG_SEVERITY_LOW:          
            strcpy(severity_str, "low");          
            g_severity = G_DEBUG_MESSAGE_SEVERITY_WARNING;
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: 
            strcpy(severity_str, "notification"); 
            g_severity = G_DEBUG_MESSAGE_SEVERITY_NOTIFY;
            break;
    }

    auto context = (OpenGL45*)userParam;
    context->_api->_send_debug_message(g_severity, "somewhere in opengl",
        "\nOpenGL debug message\n"
        "Message: '%s'\n"
        "Severity: %s  \n"
        "Type:     %s  \n"
        "Source:   %s  \n",
        message, severity_str, type_str, source_str);
}

GLenum OpenGL45::convert_shader_source_type(graphics_enum_t shader_source_type) {
    switch (shader_source_type)
    {
    case G_SHADER_SOURCE_TYPE_FRAGMENT: return GL_FRAGMENT_SHADER;
    case G_SHADER_SOURCE_TYPE_VERTEX:   return GL_VERTEX_SHADER;
    default:
        report_invalid_enum(shader_source_type);
        return 0;
    }
}

GLenum OpenGL45::convert_buffer_usage(graphics_enum_t usage) {
    switch (usage)
    {
    case G_BUFFER_USAGE_DYNAMIC_COPY:  return GL_DYNAMIC_COPY;
    case G_BUFFER_USAGE_DYNAMIC_READ:  return GL_DYNAMIC_READ;
    case G_BUFFER_USAGE_DYNAMIC_WRITE: return GL_DYNAMIC_DRAW;
    case G_BUFFER_USAGE_STREAM_COPY:   return GL_STREAM_COPY;
    case G_BUFFER_USAGE_STREAM_READ:   return GL_STREAM_READ;
    case G_BUFFER_USAGE_STREAM_WRITE:  return GL_STREAM_DRAW;
    case G_BUFFER_USAGE_STATIC_COPY:   return GL_STATIC_COPY;
    case G_BUFFER_USAGE_STATIC_READ:   return GL_STATIC_READ;
    case G_BUFFER_USAGE_STATIC_WRITE:  return GL_STATIC_DRAW;
    default:
        report_invalid_enum(usage);
        return 0;
    }
}

GLenum OpenGL45::convert_data_type_to_shader_type(graphics_enum_t data_type) {
    switch (data_type)
    {
        case G_DATA_TYPE_U8:
        case G_DATA_TYPE_U8VEC2:
        case G_DATA_TYPE_U8VEC3:
        case G_DATA_TYPE_U8VEC4:
            return GL_UNSIGNED_BYTE;

        case G_DATA_TYPE_S8:
        case G_DATA_TYPE_S8VEC2:
        case G_DATA_TYPE_S8VEC3:
        case G_DATA_TYPE_S8VEC4:
            return GL_BYTE;

        case G_DATA_TYPE_U16:
        case G_DATA_TYPE_U16VEC2:
        case G_DATA_TYPE_U16VEC3:
        case G_DATA_TYPE_U16VEC4:
            return GL_UNSIGNED_SHORT;

        case G_DATA_TYPE_S16VEC2:
        case G_DATA_TYPE_S16VEC3:
        case G_DATA_TYPE_S16VEC4:
        case G_DATA_TYPE_S16:
            return GL_SHORT;

        case G_DATA_TYPE_U32:
        case G_DATA_TYPE_U32VEC2:
        case G_DATA_TYPE_U32VEC3:
        case G_DATA_TYPE_U32VEC4:
            return GL_UNSIGNED_INT;

        case G_DATA_TYPE_S32VEC2:
        case G_DATA_TYPE_S32VEC3:
        case G_DATA_TYPE_S32VEC4:
        case G_DATA_TYPE_S32:
        case G_DATA_TYPE_TEXTURE:
            return GL_INT;
                                  
        case G_DATA_TYPE_F32:
        case G_DATA_TYPE_F32VEC3:
        case G_DATA_TYPE_F32VEC2:
        case G_DATA_TYPE_F32VEC4:
        case G_DATA_TYPE_F32MAT4:
            return GL_FLOAT;

        case G_DATA_TYPE_F64:
        case G_DATA_TYPE_F64VEC2:
        case G_DATA_TYPE_F64VEC3:
        case G_DATA_TYPE_F64VEC4:
            return GL_DOUBLE;
            
    default:
        report_invalid_enum(data_type);
        return 0;
    }
}

GLenum OpenGL45::convert_draw_mode(graphics_enum_t draw_mode) {
    switch (draw_mode) {
        case G_DRAW_MODE_POINTS:                   return GL_POINTS;
        case G_DRAW_MODE_LINE_STRIP:               return GL_LINE_STRIP;
        case G_DRAW_MODE_LINE_LOOP:                return GL_LINE_LOOP;
        case G_DRAW_MODE_LINES:                    return GL_LINES;
        case G_DRAW_MODE_LINE_STRIP_ADJACENCY:     return GL_LINE_STRIP_ADJACENCY;
        case G_DRAW_MODE_LINES_ADJACENCY:          return GL_LINES_ADJACENCY;
        case G_DRAW_MODE_TRIANGLE_STRIP:           return GL_TRIANGLE_STRIP;
        case G_DRAW_MODE_TRIANGLE_FAN:             return GL_TRIANGLE_FAN;
        case G_DRAW_MODE_TRIANGLES:                return GL_TRIANGLES;
        case G_DRAW_MODE_TRIANGLE_STRIP_ADJACENCY: return GL_TRIANGLE_STRIP_ADJACENCY;
        case G_DRAW_MODE_TRIANGLES_ADJACENCY:      return GL_TRIANGLES_ADJACENCY;
        case G_DRAW_MODE_PATCHES:                  return GL_PATCHES;

        default:
            report_invalid_enum(draw_mode);
            return 0;
    }
}

GLenum OpenGL45::convert_texture_wrapping(graphics_enum_t value) {
    switch (value) {
        case G_WRAP_REPEAT:                        return GL_REPEAT;
        case G_WRAP_MIRRORED_REPEAT:               return GL_MIRRORED_REPEAT;
        case G_WRAP_CLAMP_TO_EDGE:                 return GL_CLAMP_TO_EDGE;
        case G_WRAP_CLAMP_TO_BORDER:               return GL_CLAMP_TO_BORDER;
        default:
            report_invalid_enum(value);
            return 0;
    }
}

GLenum OpenGL45::convert_texture_filtering(graphics_enum_t value) {
    switch (value) {
        case G_MIN_FILTER_NEAREST:                 return GL_NEAREST;
        case G_MIN_FILTER_LINEAR:                  return GL_LINEAR;
        case G_MIN_FILTER_NEAREST_MIPMAP_NEAREST:  return GL_NEAREST_MIPMAP_NEAREST;
        case G_MIN_FILTER_NEAREST_MIPMAP_LINEAR:   return GL_NEAREST_MIPMAP_LINEAR;
        case G_MIN_FILTER_LINEAR_MIPMAP_NEAREST:   return GL_LINEAR_MIPMAP_NEAREST;
        case G_MIN_FILTER_LINEAR_MIPMAP_LINEAR:    return GL_LINEAR_MIPMAP_LINEAR;
        case G_MAG_FILTER_NEAREST:                 return GL_NEAREST;
        case G_MAG_FILTER_LINEAR:                  return GL_LINEAR;
        default:
            report_invalid_enum(value);
            return 0;
    }
}

GLenum OpenGL45::convert_texture_format(graphics_enum_t fmt) {
    switch (fmt) {
        case G_TEXTURE_FORMAT_RED:          return GL_RED;
        case G_TEXTURE_FORMAT_RG:           return GL_RG;
        case G_TEXTURE_FORMAT_RGB:          return GL_RGB;
        case G_TEXTURE_FORMAT_BGR:          return GL_BGR;
        case G_TEXTURE_FORMAT_RGBA:         return GL_RGBA;
        case G_TEXTURE_FORMAT_BGRA:         return GL_BGRA;
        case G_TEXTURE_FORMAT_RED_INTEGER:  return GL_RED_INTEGER;
        case G_TEXTURE_FORMAT_RG_INTEGER:   return GL_RG_INTEGER;
        case G_TEXTURE_FORMAT_RGB_INTEGER:  return GL_RGB_INTEGER;
        case G_TEXTURE_FORMAT_BGR_INTEGER:  return GL_BGR_INTEGER;
        case G_TEXTURE_FORMAT_RGBA_INTEGER: return GL_RGBA_INTEGER;
        case G_TEXTURE_FORMAT_BGRA_INTEGER: return GL_BGRA_INTEGER;
        default:
            report_invalid_enum(fmt);
            return 0;
    }
}

GLenum OpenGL45::convert_texture_slot(graphics_enum_t slot) {
    switch (slot) {
        case G_TEXTURE_SLOT_0 : return GL_TEXTURE0 ;
        case G_TEXTURE_SLOT_1 : return GL_TEXTURE1 ;
        case G_TEXTURE_SLOT_2 : return GL_TEXTURE2 ;
        case G_TEXTURE_SLOT_3 : return GL_TEXTURE3 ;
        case G_TEXTURE_SLOT_4 : return GL_TEXTURE4 ;
        case G_TEXTURE_SLOT_5 : return GL_TEXTURE5 ;
        case G_TEXTURE_SLOT_6 : return GL_TEXTURE6 ;
        case G_TEXTURE_SLOT_7 : return GL_TEXTURE7 ;
        case G_TEXTURE_SLOT_8 : return GL_TEXTURE8 ;
        case G_TEXTURE_SLOT_9 : return GL_TEXTURE9 ;
        case G_TEXTURE_SLOT_10: return GL_TEXTURE10;
        case G_TEXTURE_SLOT_11: return GL_TEXTURE11;
        case G_TEXTURE_SLOT_12: return GL_TEXTURE12;
        case G_TEXTURE_SLOT_13: return GL_TEXTURE13;
        case G_TEXTURE_SLOT_14: return GL_TEXTURE14;
        case G_TEXTURE_SLOT_15: return GL_TEXTURE15;
        case G_TEXTURE_SLOT_16: return GL_TEXTURE16;
        case G_TEXTURE_SLOT_17: return GL_TEXTURE17;
        case G_TEXTURE_SLOT_18: return GL_TEXTURE18;
        case G_TEXTURE_SLOT_19: return GL_TEXTURE19;
        case G_TEXTURE_SLOT_20: return GL_TEXTURE20;
        case G_TEXTURE_SLOT_21: return GL_TEXTURE21;
        case G_TEXTURE_SLOT_22: return GL_TEXTURE22;
        case G_TEXTURE_SLOT_23: return GL_TEXTURE23;
        case G_TEXTURE_SLOT_24: return GL_TEXTURE24;
        case G_TEXTURE_SLOT_25: return GL_TEXTURE25;
        case G_TEXTURE_SLOT_26: return GL_TEXTURE26;
        case G_TEXTURE_SLOT_27: return GL_TEXTURE27;
        case G_TEXTURE_SLOT_28: return GL_TEXTURE28;
        case G_TEXTURE_SLOT_29: return GL_TEXTURE29;
        case G_TEXTURE_SLOT_30: return GL_TEXTURE30;
        case G_TEXTURE_SLOT_31: return GL_TEXTURE31;
        default:
            report_invalid_enum(slot);
            return 0;
    }
}

void OpenGL45::init(bool show_window) {

    glfwMakeContextCurrent(NULL);
    //if (!show_window)
    //    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _AP_ENABLE_GL_DEBUG_CONTEXT
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
    mz::ivec2 sz = show_window ? mz::ivec2(1280, 720) : mz::ivec2(1, 1);
    auto main_window = glfwCreateWindow(sz.x, sz.y, "Apparatus Window", NULL, NULL);

    ap_assert(main_window, "Failed creating GLFW window in OpenGL context");

    glfwMakeContextCurrent(main_window);

    glfwSetWindowUserPointer(main_window, &_windows_context);

    auto& info = _windows_context.window_info[main_window];
    info.size = { 1280, 720 };
    info.title = "Apparatus Window";
    glfwGetWindowPos((GLFWwindow*)main_window, &info.pos.x, &info.pos.y);

    auto result = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    ap_assert(result, "Failed loading GL with GLAD. Error code: {}", result);
    (void)result;

    _windows_context.main_window_handle = main_window;
    _windows_context.__should_close = [this](void* wnd) { 
        int ret = 0;
        _api->_thread_server->queue_task(_api->_tid, [&ret, wnd]() {
            glfwMakeContextCurrent((GLFWwindow*)wnd);
            ret = glfwWindowShouldClose((GLFWwindow*)wnd);
        });
        _api->_thread_server->wait_for_thread(_api->_tid);
        return ret; 
    };
    _windows_context.__swap_buffers = [this](void* wnd) { 
        _api->_thread_server->queue_task(_api->_tid, [this, wnd]() {
            glfwMakeContextCurrent((GLFWwindow*)wnd);
            glfwSwapBuffers((GLFWwindow*)wnd);
            glfwPollEvents();

            f64 time = glfwGetTime();
            auto& wnd_info = _windows_context.window_info[wnd];
            wnd_info.delta_time = time > 0 ? time - wnd_info._last_time : (double)1 / (double)60;
            wnd_info._last_time = time;
        });
    };
    _windows_context.__is_key_down = [this](void* wnd, input_code_t code) {
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer((GLFWwindow*)wnd);
        auto& info = windows_context->window_info[wnd];

        std::lock_guard<std::mutex> l(info._input_mutex);
        return info._input.keys_down[code];
    };
    _windows_context.__is_key_pressed = [this](void* wnd, input_code_t code) {
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer((GLFWwindow*)wnd);
        auto& info = windows_context->window_info[wnd];

        std::lock_guard<std::mutex> l(info._input_mutex);
        return info._input.keys_press[code];
    };
    _windows_context.__get_mouse_position = [this](void* wnd) {
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer((GLFWwindow*)wnd);
        auto& info = windows_context->window_info[wnd];

        std::lock_guard<std::mutex> l(info._input_mutex);
        return info._input.mouse_pos;
        
    }; 
    _windows_context.__is_mouse_down = [this](void* wnd, input_code_t code) {
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer((GLFWwindow*)wnd);
        auto& info = windows_context->window_info[wnd];

        std::lock_guard<std::mutex> l(info._input_mutex);
        return info._input.mouse_down[code];
    };
    _windows_context.__is_mouse_pressed = [this](void* wnd, input_code_t code) {
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer((GLFWwindow*)wnd);
        auto& info = windows_context->window_info[wnd];

        std::lock_guard<std::mutex> l(info._input_mutex);
        return info._input.mouse_press[code];
    };
    _windows_context.__set_vsync = [this](void* wnd, bool value) {
        _api->_thread_server->queue_task(_api->_tid, [this, wnd, value]() {
            glfwMakeContextCurrent((GLFWwindow*)wnd);
            glfwSwapInterval((int)value);
        });
    }; 

    _windows_context.__hide_window = [this](void* wnd) {
        _api->_thread_server->queue_task(_api->_tid, [this, wnd]() {
            glfwMakeContextCurrent((GLFWwindow*)wnd);
            glfwHideWindow((GLFWwindow*)wnd);
        });
    }; 
    _windows_context.__show_window = [this](void* wnd) {
        _api->_thread_server->queue_task(_api->_tid, [this, wnd]() {
            glfwMakeContextCurrent((GLFWwindow*)wnd);
            glfwShowWindow((GLFWwindow*)wnd);
        });
    }; 

    glfwSetWindowSizeCallback(main_window, [](GLFWwindow* wnd, int x, int y) {
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer(wnd);

        windows_context->window_info[wnd].size = { x, y };

        glViewport(0, 0, x, y);
    });

    glfwSetWindowPosCallback(main_window, [](GLFWwindow* wnd, int x, int y) {
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer(wnd);

        windows_context->window_info[wnd].pos = { x, y };
    });

    glfwSetKeyCallback(main_window, [](GLFWwindow* wnd, int code, int scancode, int state, int mods) {
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        (void)scancode;(void)mods;
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer(wnd);

        auto& input = windows_context->window_info[wnd]._input;

        bool is_now_down = state == GLFW_PRESS;
        input.keys_press[code] = is_now_down && !input.keys_down[code];
        input.keys_down[code] = is_now_down;
    });

    glfwSetMouseButtonCallback(main_window, [](GLFWwindow* wnd, int code, int state, int mods) {
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        (void)mods;
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer(wnd);

        auto& input = windows_context->window_info[wnd]._input;

        bool is_now_down = state == GLFW_PRESS;
        input.mouse_press[code] = is_now_down && !input.mouse_down[code];
        input.mouse_down[code] = is_now_down;
    });

    glfwSetCursorPosCallback(main_window, [](GLFWwindow* wnd, double x, double y){
        glfwMakeContextCurrent((GLFWwindow*)wnd);
        auto* windows_context = (Windows_Context*)glfwGetWindowUserPointer(wnd);

        auto& input = windows_context->window_info[wnd]._input;

        input.mouse_pos = mz::fvec2((f32)x, (f32)y);
    });

    _windows_context.set_vsync(main_window, false);
    if (show_window) _windows_context.show_window(main_window);
    else             _windows_context.hide_window(main_window);

#ifdef _AP_ENABLE_GL_DEBUG_CONTEXT
    GLint ctx_flags; 
    glGetIntegerv(GL_CONTEXT_FLAGS, &ctx_flags);
    if (ctx_flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(glDebugOutput, this);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    _imgui_context = ImGui::CreateContext();
    ap_assert(_imgui_context, "Failed creating imgui context");

    this->use_imgui_context();

    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGuiStyle& style = ImGui::GetStyle();

    //ImGui::StyleColorsDark();
    /*style.Colors[ImGuiCol_Text] = ImVec4(0.31f, 0.25f, 0.24f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.74f, 0.74f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.68f, 0.68f, 0.68f, 0.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.62f, 0.70f, 0.72f, 0.56f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.33f, 0.14f, 0.47f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.97f, 0.31f, 0.13f, 0.81f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.42f, 0.75f, 1.00f, 0.53f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.65f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.47f, 0.47f, 0.71f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.00f, 0.79f, 0.18f, 0.78f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.42f, 0.82f, 1.00f, 0.81f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.72f, 1.00f, 1.00f, 0.86f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.78f, 0.84f, 0.80f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.68f, 0.74f, 0.80f);//ImVec4(0.46f, 0.84f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.80f, 0.30f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.99f, 0.54f, 0.43f);
    style.Alpha = 1.0f;
    style.FrameRounding = 4;
    style.IndentSpacing = 12.0f;*/

    /*style.WindowRounding = 5.3f;
    style.GrabRounding = style.FrameRounding = 2.3f;
    style.ScrollbarRounding = 5.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing.y = 6.5f;

    style.Colors[ImGuiCol_Text]                  = {0.73333335f, 0.73333335f, 0.73333335f, 1.00f};
    style.Colors[ImGuiCol_TextDisabled]          = {0.34509805f, 0.34509805f, 0.34509805f, 1.00f};
    style.Colors[ImGuiCol_WindowBg]              = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
    style.Colors[ImGuiCol_ChildBg]               = {0.23529413f, 0.24705884f, 0.25490198f, 0.00f};
    style.Colors[ImGuiCol_PopupBg]               = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
    style.Colors[ImGuiCol_Border]                = {0.33333334f, 0.33333334f, 0.33333334f, 0.50f};
    style.Colors[ImGuiCol_BorderShadow]          = {0.15686275f, 0.15686275f, 0.15686275f, 0.00f};
    style.Colors[ImGuiCol_FrameBg]               = {0.16862746f, 0.16862746f, 0.16862746f, 0.54f};
    style.Colors[ImGuiCol_FrameBgHovered]        = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
    style.Colors[ImGuiCol_FrameBgActive]         = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
    style.Colors[ImGuiCol_Tab]                   = {0.16f, 0.29f, 0.48f, 1.00f};
    style.Colors[ImGuiCol_TabActive]             = {0.36f, 0.49f, 0.68f, 1.00f};
    style.Colors[ImGuiCol_TabHovered]            = {0.26f, 0.39f, 0.58f, 1.00f};
    style.Colors[ImGuiCol_TabUnfocused]          = {0.06f, 0.39f, 0.38f, 1.00f};
    style.Colors[ImGuiCol_TabUnfocusedActive]    = {0.46f, 0.59f, 0.58f, 1.00f};
    style.Colors[ImGuiCol_TitleBg]               = {0.04f, 0.04f, 0.04f, 1.00f};
    style.Colors[ImGuiCol_TitleBgCollapsed]      = {0.16f, 0.29f, 0.48f, 1.00f};
    style.Colors[ImGuiCol_TitleBgActive]         = {0.00f, 0.00f, 0.00f, 0.51f};
    style.Colors[ImGuiCol_MenuBarBg]             = {0.27058825f, 0.28627452f, 0.2901961f, 0.80f};
    style.Colors[ImGuiCol_ScrollbarBg]           = {0.27058825f, 0.28627452f, 0.2901961f, 0.60f};
    style.Colors[ImGuiCol_ScrollbarGrab]         = {0.21960786f, 0.30980393f, 0.41960788f, 0.51f};
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = {0.13725491f, 0.19215688f, 0.2627451f, 0.91f};
    style.Colors[ImGuiCol_CheckMark]             = {0.90f, 0.90f, 0.90f, 0.83f};
    style.Colors[ImGuiCol_SliderGrab]            = {0.70f, 0.70f, 0.70f, 0.62f};
    style.Colors[ImGuiCol_SliderGrabActive]      = {0.30f, 0.30f, 0.30f, 0.84f};
    style.Colors[ImGuiCol_Button]                = {0.33333334f, 0.3529412f, 0.36078432f, 0.49f};
    style.Colors[ImGuiCol_ButtonHovered]         = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
    style.Colors[ImGuiCol_ButtonActive]          = {0.13725491f, 0.19215688f, 0.2627451f, 1.00f};
    style.Colors[ImGuiCol_Header]                = {0.33333334f, 0.3529412f, 0.36078432f, 0.53f};
    style.Colors[ImGuiCol_HeaderHovered]         = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
    style.Colors[ImGuiCol_HeaderActive]          = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
    style.Colors[ImGuiCol_Separator]             = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
    style.Colors[ImGuiCol_SeparatorHovered]      = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
    style.Colors[ImGuiCol_SeparatorActive]       = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
    style.Colors[ImGuiCol_ResizeGrip]            = {1.00f, 1.00f, 1.00f, 0.85f};
    style.Colors[ImGuiCol_ResizeGripHovered]     = {1.00f, 1.00f, 1.00f, 0.60f};
    style.Colors[ImGuiCol_ResizeGripActive]      = {1.00f, 1.00f, 1.00f, 0.90f};
    style.Colors[ImGuiCol_PlotLines]             = {0.61f, 0.61f, 0.61f, 1.00f};
    style.Colors[ImGuiCol_PlotLinesHovered]      = {1.00f, 0.43f, 0.35f, 1.00f};
    style.Colors[ImGuiCol_PlotHistogram]         = {0.90f, 0.70f, 0.00f, 1.00f};
    style.Colors[ImGuiCol_PlotHistogramHovered]  = {1.00f, 0.60f, 0.00f, 1.00f};
    style.Colors[ImGuiCol_TextSelectedBg]        = {0.18431373f, 0.39607847f, 0.79215693f, 0.90f};*/

    constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b)
    {
        return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
    };

    ImVec4* colors = style.Colors;

    const ImVec4 bgColor           = ColorFromBytes(37, 37, 38);
    const ImVec4 lightBgColor      = ColorFromBytes(82, 82, 85);
    const ImVec4 veryLightBgColor  = ColorFromBytes(90, 90, 95);

    const ImVec4 panelColor        = ColorFromBytes(51, 51, 55);
    const ImVec4 panelHoverColor   = ColorFromBytes(29, 151, 236);
    const ImVec4 panelActiveColor  = ColorFromBytes(0, 119, 200);

    const ImVec4 textColor         = ColorFromBytes(255, 255, 255);
    const ImVec4 textDisabledColor = ColorFromBytes(151, 151, 151);
    const ImVec4 borderColor       = ColorFromBytes(78, 78, 78);

    colors[ImGuiCol_Text]                 = textColor;
    colors[ImGuiCol_TextDisabled]         = textDisabledColor;
    colors[ImGuiCol_TextSelectedBg]       = panelActiveColor;
    colors[ImGuiCol_WindowBg]             = bgColor;
    colors[ImGuiCol_ChildBg]              = bgColor;
    colors[ImGuiCol_PopupBg]              = bgColor;
    colors[ImGuiCol_Border]               = borderColor;
    colors[ImGuiCol_BorderShadow]         = borderColor;
    colors[ImGuiCol_FrameBg]              = panelColor;
    colors[ImGuiCol_FrameBgHovered]       = panelHoverColor;
    colors[ImGuiCol_FrameBgActive]        = panelActiveColor;
    colors[ImGuiCol_TitleBg]              = bgColor;
    colors[ImGuiCol_TitleBgActive]        = bgColor;
    colors[ImGuiCol_TitleBgCollapsed]     = bgColor;
    colors[ImGuiCol_MenuBarBg]            = panelColor;
    colors[ImGuiCol_ScrollbarBg]          = panelColor;
    colors[ImGuiCol_ScrollbarGrab]        = lightBgColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
    colors[ImGuiCol_ScrollbarGrabActive]  = veryLightBgColor;
    colors[ImGuiCol_CheckMark]            = panelActiveColor;
    colors[ImGuiCol_SliderGrab]           = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive]     = panelActiveColor;
    colors[ImGuiCol_Button]               = panelColor;
    colors[ImGuiCol_ButtonHovered]        = panelHoverColor;
    colors[ImGuiCol_ButtonActive]         = panelHoverColor;
    colors[ImGuiCol_Header]               = panelColor;
    colors[ImGuiCol_HeaderHovered]        = panelHoverColor;
    colors[ImGuiCol_HeaderActive]         = panelActiveColor;
    colors[ImGuiCol_Separator]            = borderColor;
    colors[ImGuiCol_SeparatorHovered]     = borderColor;
    colors[ImGuiCol_SeparatorActive]      = borderColor;
    colors[ImGuiCol_ResizeGrip]           = bgColor;
    colors[ImGuiCol_ResizeGripHovered]    = panelColor;
    colors[ImGuiCol_ResizeGripActive]     = lightBgColor;
    colors[ImGuiCol_PlotLines]            = panelActiveColor;
    colors[ImGuiCol_PlotLinesHovered]     = panelHoverColor;
    colors[ImGuiCol_PlotHistogram]        = panelActiveColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
    colors[ImGuiCol_ModalWindowDarkening] = ImVec4(bgColor.x, bgColor.y, bgColor.z, .5f);
    colors[ImGuiCol_DragDropTarget]       = bgColor;
    colors[ImGuiCol_NavHighlight]         = bgColor;
    colors[ImGuiCol_DockingPreview]       = panelActiveColor;
    colors[ImGuiCol_Tab]                  = bgColor;
    colors[ImGuiCol_TabActive]            = panelActiveColor;
    colors[ImGuiCol_TabUnfocused]         = bgColor;
    colors[ImGuiCol_TabUnfocusedActive]   = panelActiveColor;
    colors[ImGuiCol_TabHovered]           = panelHoverColor;

    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 0.0f;
    style.FrameRounding     = 0.0f;
    style.GrabRounding      = 0.0f;
    style.PopupRounding     = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.TabRounding       = 0.0f;

    bool glfw_init_result = ImGui_ImplGlfw_InitForOpenGL(main_window, true);
    ap_assert(glfw_init_result, "Failed to initialize ImGui for glfw");
    (void)glfw_init_result;

    bool opengl_init_result = ImGui_ImplOpenGL3_Init("#version 410");
    ap_assert(opengl_init_result, "Failed to initialize ImGui for OpenGL3");
    (void)opengl_init_result;

    glLineWidth(2);
}

OpenGL45::~OpenGL45() {
    _api->_thread_server->queue_task(_api->_tid, [this]() {
        glfwMakeContextCurrent((GLFWwindow*)_windows_context.main_window_handle);
        ImGui::SetCurrentContext(_imgui_context);
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
        
        for (auto* wnd : _windows_context.window_handles) {
            glfwDestroyWindow((GLFWwindow*)wnd);
        }
        glfwDestroyWindow((GLFWwindow*)_windows_context.main_window_handle);
    });

    _api->_thread_server->wait_for_thread(_api->_tid);
}

void OpenGL45::set_culling(graphics_enum_t value) {
    make_context_current();
    switch (value)
    {
    case G_CULL_FRONT:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Face culling set to front");
        break;
    case G_CULL_BACK:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Face culling set to back");
        break;
    case G_CULL_NONE:
        glDisable(GL_CULL_FACE);
        send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Face culling disabled");
        break;
    default:
        report_invalid_enum(value);
        break;
    }
}

void OpenGL45::set_blending(bool value) {
    make_context_current();
    if (value) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Enabled blending");
    } else {
        glDisable(GL_BLEND);

        send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Disabled blending");
    }
}

graphics_id_t OpenGL45::make_vertex_array(const Buffer_Layout_Specification& layout) {
    make_context_current();
    GLuint va;
    glGenVertexArrays(1, &va);

    if (va >= _buffer_layouts.size()) {
        _buffer_layouts.resize((size_t)(va + 1));
    }

    _buffer_layouts[va] = layout;

    return (graphics_id_t)va;
}

graphics_id_t OpenGL45::make_shader_source(graphics_enum_t source_type, const char* src) {
    make_context_current();
    GLuint shader_source = glCreateShader(convert_shader_source_type(source_type));
    
    glShaderSource(shader_source, 1, &src, NULL);

    glCompileShader(shader_source);
    GLint compile_result;
    glGetShaderiv(shader_source, GL_COMPILE_STATUS, &compile_result);
    if (compile_result == GL_FALSE) {
        GLint log_len;
        char log[1024] = "";

        glGetShaderiv(shader_source, GL_INFO_LOG_LENGTH, &log_len);
        glGetShaderInfoLog(shader_source, log_len, &log_len, log);

        send_message(G_DEBUG_MESSAGE_SEVERITY_ERROR,
                     "Failed compiling shader source '%u' of type %s.\nCompilation output:\n%s", 
                     shader_source, get_graphics_enum_string(source_type), log);
        return 0;
    }

    return shader_source;
}

graphics_id_t OpenGL45::make_shader(graphics_id_t vert_shader, graphics_id_t frag_shader, const Buffer_Layout_Specification& input_layout) {
    make_context_current();
    GLuint program = glCreateProgram();
    _shader_input_layouts.resize(std::max<graphics_id_t>(program + 1, (graphics_id_t)_shader_input_layouts.size()));
    _shader_input_layouts[program] = input_layout;

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    GLint link_result;
    glGetProgramiv(program, GL_LINK_STATUS, &link_result);

    if (link_result == GL_FALSE) {
        char link_log[1024];
        glGetProgramInfoLog(program, 1024, NULL, link_log);

        send_message(G_DEBUG_MESSAGE_SEVERITY_ERROR, "Failed linking vert & frag sources %u & %u into program %u.\nlog:\n%s", program, vert_shader, frag_shader, link_log);

        glDeleteProgram(program);
        return 0;
    }

    glValidateProgram(program);

    static int samplers[32] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 
                                14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                26, 27, 28, 29, 30, 31 };
    glUseProgram(program);
    glUniform1iv(glGetUniformLocation(program, "samplers"), 32, samplers);
    glUseProgram(0);

    return program;
}   

graphics_id_t OpenGL45::make_vertex_buffer(void* data, size_t sz, graphics_enum_t usage) {
    make_context_current();
    GLuint vb;
    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, sz, data, convert_buffer_usage(usage));

    send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Created a GL vertex buffer with id %u", vb);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vb;
}

graphics_id_t OpenGL45::make_index_buffer(u32* indices, count_t count, graphics_enum_t usage) {
    make_context_current();
    GLuint ib;
    glGenBuffers(1, &ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), indices, convert_buffer_usage(usage));

    _index_count.resize(std::max<graphics_id_t>(ib + 1, (graphics_id_t)_index_count.size()));
    _index_count[ib] = count;

    send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Created a GL index buffer with id %u", ib);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return ib;
}

graphics_id_t OpenGL45::make_uniform_buffer(void* data, const Buffer_Layout_Specification& layout, graphics_enum_t usage) {
    make_context_current();
    
    GLuint ub;
    glGenBuffers(1, &ub);
    _uniform_buffer_layouts.resize(std::max<graphics_id_t>(ub + 1, (graphics_id_t)_uniform_buffer_layouts.size()));
    _uniform_buffer_layouts[ub] = layout;
    _uniform_buffer_layouts[ub].align_each_entry(16);
    glBindBuffer(GL_UNIFORM_BUFFER, ub);
    glBufferData(
        GL_UNIFORM_BUFFER, 
        _uniform_buffer_layouts[ub].stride, 
        data, 
        convert_buffer_usage(usage)
    );

    send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Created a GL uniform buffer with id %u", ub);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ub, 0, _uniform_buffer_layouts[ub].stride);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return ub;
}

graphics_id_t OpenGL45::make_texture(graphics_enum_t usage) {
    (void)usage;
    make_context_current();

    send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Making texture");
    GLuint texture;
    glGenTextures(1, &texture);
    send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Made texture {}", texture);

    return texture;
}

graphics_id_t OpenGL45::make_render_target(mz::ivec2 size) {
    graphics_id_t fbo = G_NULL_ID;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    graphics_id_t texture = G_NULL_ID;
    glGenTextures(1, &texture);

    _render_targets.resize(std::max<graphics_id_t>(fbo + 1, (graphics_id_t)_render_targets.size()));

    _render_targets[fbo].fbo = fbo;
    _render_targets[fbo].size = size;
    _render_targets[fbo].texture = texture;

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        send_message(G_DEBUG_MESSAGE_SEVERITY_CRITICAL, "Failed creating framebuffer");
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Created a framebuffer with size { %i, %i } (id %u)", size.x, size.y, fbo);

    return fbo;
}

void OpenGL45::destroy_vertex_array(graphics_id_t vao) {
    make_context_current();
    glDeleteVertexArrays(1, &vao);
}

void OpenGL45::destroy_shader(graphics_id_t shader){
    make_context_current();
    glDeleteProgram(shader);
}

void OpenGL45::destroy_shader_source(graphics_id_t shader_source) {
    glDeleteShader(shader_source);
}

void OpenGL45::destroy_vertex_buffer(graphics_id_t vbo){
    make_context_current();
    glDeleteBuffers(1, &vbo);
}

void OpenGL45::destroy_index_buffer(graphics_id_t ibo){
    make_context_current();
    glDeleteBuffers(1, &ibo);
}

void OpenGL45::destroy_uniform_buffer(graphics_id_t ubo) {
    make_context_current();
    glDeleteBuffers(1, &ubo);
}

void OpenGL45::destroy_texture(graphics_id_t texture) {
    make_context_current();

    glDeleteTextures(1, &texture);
}

void OpenGL45::destroy_render_target(graphics_id_t render_target) {
    auto& fbo_info = _render_targets[render_target];
    send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Destroying render target %u", render_target);
    glDeleteTextures(1, &fbo_info.texture);
    glDeleteFramebuffers(1, &fbo_info.fbo);
}

void OpenGL45::set_texture_data(graphics_id_t texture, byte* data, mz::ivec2 size, graphics_enum_t fmt) {
    make_context_current();

    GLenum gl_fmt = convert_texture_format(fmt);

    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        gl_fmt, 
        size.width, 
        size.height, 
        0, 
        gl_fmt, 
        GL_UNSIGNED_BYTE, 
        data
    );

    glBindTexture(GL_TEXTURE_2D, 0);
}
void OpenGL45::set_texture_wrapping(graphics_id_t texture, graphics_enum_t wrap_mode) {
    make_context_current();

    GLenum gl_wrap = convert_texture_wrapping(wrap_mode);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wrap);

    glBindTexture(GL_TEXTURE_2D, 0);
}
void OpenGL45::set_texture_filtering(graphics_id_t texture, graphics_enum_t min_filter_mode, graphics_enum_t mag_filter_mode) {
    make_context_current();

    auto gl_min_filter = convert_texture_filtering(min_filter_mode);
    auto gl_mag_filter = convert_texture_filtering(mag_filter_mode);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mag_filter);

    glBindTexture(GL_TEXTURE_2D, 0);
}
void OpenGL45::set_texture_multisampling(graphics_id_t texture, u32 count, u32 quality) {
    (void)texture;(void)count;(void)quality;
}

void OpenGL45::bind_texture_to_slot(graphics_id_t texture, graphics_enum_t slot) {
    glActiveTexture(convert_texture_slot(slot));
    glBindTexture(GL_TEXTURE_2D, texture);
}

void OpenGL45::set_render_target_size(graphics_id_t render_target, mz::ivec2 size) {
    glBindFramebuffer(GL_FRAMEBUFFER, render_target);

    auto& fbo_info = _render_targets[render_target];

    glBindTexture(GL_TEXTURE_2D, fbo_info.texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width, size.height, 0, GL_RGBA, GL_FLOAT, NULL);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        send_message(G_DEBUG_MESSAGE_SEVERITY_CRITICAL, "Failed resizing framebuffer");
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Resized a framebuffer to size { %i, %i }", size.x, size.y);

    fbo_info.size = size;
}
mz::ivec2 OpenGL45::get_render_target_size(graphics_id_t render_target) {
    return _render_targets[render_target].size;
}

graphics_id_t OpenGL45::get_render_target_texture(graphics_id_t render_target) {
    return _render_targets[render_target].texture;
}

void* OpenGL45::get_native_texture_handle(graphics_id_t texture) {
    return (void*)(uintptr_t)texture;
}

void OpenGL45::set_clear_color(mz::color16 color, graphics_id_t render_target) {
    make_context_current();

    if (render_target != G_NULL_ID) glBindFramebuffer(GL_FRAMEBUFFER, render_target);
    glClearColor(color.r, color.g, color.b, color.a);    
    if (render_target != G_NULL_ID) glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGL45::set_viewport(mz::viewport viewport) {
    make_context_current();
    glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
}

void OpenGL45::clear(graphics_enum_t clear_flags, graphics_id_t render_target) {
    make_context_current();
    GLbitfield gl_clear_flags = 0; 
    if (clear_flags & G_COLOR_BUFFER_BIT)   gl_clear_flags |= GL_COLOR_BUFFER_BIT;
    if (clear_flags & G_DEPTH_BUFFER_BIT)   gl_clear_flags |= GL_DEPTH_BUFFER_BIT;
    if (clear_flags & G_STENCIL_BUFFER_BIT) gl_clear_flags |= GL_STENCIL_BUFFER_BIT;

    if (render_target != G_NULL_ID) glBindFramebuffer(GL_FRAMEBUFFER, render_target);
    glClear(gl_clear_flags);
    if (render_target != G_NULL_ID) glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGL45::draw_indices(graphics_id_t vao, graphics_id_t shader, u32 index_count, graphics_id_t ubo, graphics_enum_t draw_mode, graphics_id_t render_target) {
    make_context_current();
    graphics_id_t ibo = _ibo_associations[vao];

    const auto& buffer_layout = _buffer_layouts[vao];
    const auto& shader_input_layout = _shader_input_layouts[shader];

    if (!shader_input_layout.is_sated_by(buffer_layout)) {
        send_message(G_DEBUG_MESSAGE_SEVERITY_CRITICAL, "Vao layout is not compatible with shader input layout. Shader input layout needs to be sated.");
    }

    glUseProgram(shader);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    if (ubo != G_NULL_ID) {
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        GLuint ub_index = glGetUniformBlockIndex(shader, "UniformBuffer");
        glUniformBlockBinding(shader, ub_index, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    }

    if (render_target != G_NULL_ID) glBindFramebuffer(GL_FRAMEBUFFER, render_target);

    glDrawElements(convert_draw_mode(draw_mode), (GLsizei)index_count, GL_UNSIGNED_INT, NULL);

    if (render_target != G_NULL_ID) glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (ubo != G_NULL_ID) {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGL45::associate_vertex_buffer(graphics_id_t vbo, graphics_id_t vao) {
    make_context_current();
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    auto& layout = _buffer_layouts[vao];

    if (layout.entries.size() == 0) {
        send_message(G_DEBUG_MESSAGE_SEVERITY_ERROR, "No valid layout specified for vao. Make sure a layout is specified before associating a vbo with a vao.");
    }

    s32 location = 0;
    for (auto& entry : layout.entries) {
        glVertexAttribPointer(
            location,
            (GLint)entry.component_count, 
            convert_data_type_to_shader_type(entry.data_type), 
            entry.normalized ? GL_TRUE : GL_FALSE, 
            (GLsizei)layout.stride, 
            (void*)entry.offset
        );
        glEnableVertexAttribArray(location++);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void OpenGL45::associate_index_buffer(graphics_id_t ibo, graphics_id_t vao) {
    make_context_current();
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    _ibo_associations.resize(std::max<graphics_id_t>(vao + 1, (graphics_id_t)_ibo_associations.size()));
    _ibo_associations[vao] = ibo;

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGL45::set_vertex_buffer_data(graphics_id_t vbo, void* data, size_t offset, size_t sz) {
    make_context_current();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)offset, (GLsizeiptr)sz, data);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void OpenGL45::set_index_buffer_data(graphics_id_t ibo, u32* indices, count_t offset, count_t count) {
    make_context_current();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(offset * sizeof(u32)), (GLsizeiptr)(count * sizeof(u32)), indices);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGL45::set_uniform_buffer_data(graphics_id_t ubo, const char* name, void* data) {
    make_context_current();

    auto& layout = _uniform_buffer_layouts[ubo];

    for (auto entry : layout.entries) {
        if (strcmp(entry.name, name) == 0) {
            glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            glBufferSubData(GL_UNIFORM_BUFFER, (GLintptr)entry.offset, (GLsizeiptr)entry.size, data);

            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            break;
        }
    }
}

void* OpenGL45::map_vertex_buffer_data(graphics_id_t vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return ptr;
}
u32* OpenGL45::map_index_buffer_data(graphics_id_t ibo) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    void* ptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return (u32*)ptr;
}
void* OpenGL45::map_uniform_buffer_data(graphics_id_t ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    void* ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return ptr;
}
void OpenGL45::unmap_vertex_buffer_data(graphics_id_t vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void OpenGL45::unmap_index_buffer_data(graphics_id_t ibo) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void OpenGL45::unmap_uniform_buffer_data(graphics_id_t ubo) {
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGL45::use_imgui_context() {
    glfwMakeContextCurrent((GLFWwindow*)_windows_context.main_window_handle);
    ImGui::SetCurrentContext(_imgui_context);
}

void OpenGL45::update_imgui() {
    this->use_imgui_context();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void OpenGL45::render_imgui() {
    this->use_imgui_context();

    auto& io = ImGui::GetIO();
    
    auto wnd = _windows_context.main_window_handle;
    io.DeltaTime = (float)_windows_context.window_info[wnd].delta_time;
    auto sz = _windows_context.window_info[wnd].size;
    io.DisplaySize = ImVec2((float)sz.x, (float)sz.y);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_ctx = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_ctx);
    }
}

#endif // AP_SUPPORT_OPENGL45