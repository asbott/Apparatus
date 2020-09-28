#include "pch.h"

#ifdef AP_SUPPORT_DX11

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define NOMINMAX

#include <Windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include <comdef.h>

#include <filesystem> // To convert wide string to narrow with std::filesystem::path().generic_string()

#include "graphics/directx11.h"

#include "graphics/graphics_debug_macros.h"

#ifdef _CONFIG_DEBUG
    #define check_result(call, err_msg, ...) {HRESULT r = call(__VA_ARGS__);\
        if (FAILED(r)) {\
        _com_error err(r);\
        LPCTSTR errMsg = err.ErrorMessage();\
        std::wstring wstr = errMsg;\
        std::string str = std::filesystem::path(wstr).generic_string();\
        send_message(G_DEBUG_MESSAGE_SEVERITY_CRITICAL, "In function call '" #call "'\n" err_msg "\nError description: %s", str.c_str());\
    }}
#else

    #define check_result(call, err_msg, ...) call(__VA_ARGS__);

#endif

input_code_t convert_key_code(int windows_key_code) {
    if (windows_key_code >= 32 && windows_key_code <= 126) return windows_key_code;

    switch (windows_key_code) {
        case VK_BACK           : return AP_KEY_BACKSPACE;
        case VK_TAB            : return AP_KEY_TAB;
        case VK_RETURN         : return AP_KEY_ENTER;
        case VK_ESCAPE         : return AP_KEY_ESCAPE;
        case VK_SPACE          : return AP_KEY_SPACE;
        case VK_HOME           : return AP_KEY_HOME;
        case VK_MENU           : return AP_KEY_HOME;
        case VK_LEFT           : return AP_KEY_LEFT;
        case VK_UP             : return AP_KEY_UP;
        case VK_RIGHT          : return AP_KEY_RIGHT;
        case VK_DOWN           : return AP_KEY_DOWN;
        case VK_PRINT          : return AP_KEY_PRINT_SCREEN;
        case VK_SNAPSHOT       : return AP_KEY_PRINT_SCREEN;
        case VK_INSERT         : return AP_KEY_INSERT;
        case VK_DELETE         : return AP_KEY_DELETE;
        case VK_LWIN           : return AP_KEY_LEFT_SUPER;
        case VK_RWIN           : return AP_KEY_RIGHT_SUPER;
        case VK_NUMPAD0        : return AP_KEY_KP_0;
        case VK_NUMPAD1        : return AP_KEY_KP_1;
        case VK_NUMPAD2        : return AP_KEY_KP_2;
        case VK_NUMPAD3        : return AP_KEY_KP_3;
        case VK_NUMPAD4        : return AP_KEY_KP_4;
        case VK_NUMPAD5        : return AP_KEY_KP_5;
        case VK_NUMPAD6        : return AP_KEY_KP_6;
        case VK_NUMPAD7        : return AP_KEY_KP_7;
        case VK_NUMPAD8        : return AP_KEY_KP_8;
        case VK_NUMPAD9        : return AP_KEY_KP_9;
        case VK_MULTIPLY       : return AP_KEY_KP_MULTIPLY;
        case VK_ADD            : return AP_KEY_KP_ADD;
        case VK_SUBTRACT       : return AP_KEY_KP_SUBTRACT;
        case VK_DECIMAL        : return AP_KEY_KP_DECIMAL;
        case VK_DIVIDE         : return AP_KEY_KP_DIVIDE;
        case VK_F1             : return AP_KEY_F1;
        case VK_F2             : return AP_KEY_F2;
        case VK_F3             : return AP_KEY_F3;
        case VK_F4             : return AP_KEY_F4;
        case VK_F5             : return AP_KEY_F5;
        case VK_F6             : return AP_KEY_F6;
        case VK_F7             : return AP_KEY_F7;
        case VK_F8             : return AP_KEY_F8;
        case VK_F9             : return AP_KEY_F9;
        case VK_F10            : return AP_KEY_F10;
        case VK_F11            : return AP_KEY_F11;
        case VK_F12            : return AP_KEY_F12;
        case VK_F13            : return AP_KEY_F13;
        case VK_F14            : return AP_KEY_F14;
        case VK_F15            : return AP_KEY_F15;
        case VK_F16            : return AP_KEY_F16;
        case VK_F17            : return AP_KEY_F17;
        case VK_F18            : return AP_KEY_F18;
        case VK_F19            : return AP_KEY_F19;
        case VK_F20            : return AP_KEY_F20;
        case VK_F21            : return AP_KEY_F21;
        case VK_F22            : return AP_KEY_F22;
        case VK_F23            : return AP_KEY_F23;
        case VK_F24            : return AP_KEY_F24;
        case VK_NUMLOCK        : return AP_KEY_NUM_LOCK;
        case VK_SCROLL         : return AP_KEY_SCROLL_LOCK;
        case VK_OEM_NEC_EQUAL  : return AP_KEY_KP_EQUAL;
        case VK_LSHIFT         : return AP_KEY_LEFT_SHIFT;
        case VK_RSHIFT         : return AP_KEY_RIGHT_SHIFT;
        case VK_LCONTROL       : return AP_KEY_LEFT_CONTROL;
        case VK_RCONTROL       : return AP_KEY_RIGHT_CONTROL;
        case VK_LMENU          : return AP_KEY_RIGHT;
        case VK_OEM_1          : return AP_KEY_SEMICOLON;
        case VK_OEM_PERIOD     : return AP_KEY_PERIOD;
        case VK_OEM_2          : return AP_KEY_BACKSLASH;
        case VK_OEM_3          : return AP_KEY_APOSTROPHE;

        default: log_trace("Unsupported windows key code key {}", windows_key_code); return 0;
    }
}

s32 DirectX11::convert_buffer_usage(graphics_enum_t usage) {
    switch (usage)
    {
    case G_BUFFER_USAGE_DYNAMIC_WRITE:
        return (s32)D3D11_USAGE_DYNAMIC;

    case G_BUFFER_USAGE_STREAM_COPY:
    case G_BUFFER_USAGE_STREAM_WRITE:
    case G_BUFFER_USAGE_STATIC_COPY:
    case G_BUFFER_USAGE_DYNAMIC_READ:
    case G_BUFFER_USAGE_STREAM_READ:    
    case G_BUFFER_USAGE_STATIC_READ:
    case G_BUFFER_USAGE_STATIC_WRITE:
    case G_BUFFER_USAGE_DYNAMIC_COPY:
        return (s32)D3D11_USAGE_DEFAULT;

    default:
        report_invalid_enum(usage);
        return 0;
    }
}

s32 DirectX11::convert_buffer_usage_cpu_access(graphics_enum_t usage) {
    switch (usage)
    {
    case G_BUFFER_USAGE_DYNAMIC_COPY:
        return (s32)D3D11_CPU_ACCESS_READ;
    
    case G_BUFFER_USAGE_DYNAMIC_WRITE:
        return (s32)D3D11_CPU_ACCESS_WRITE;

    case G_BUFFER_USAGE_STATIC_WRITE:
    case G_BUFFER_USAGE_STREAM_COPY:
    case G_BUFFER_USAGE_STREAM_WRITE:
    case G_BUFFER_USAGE_STATIC_COPY:
    case G_BUFFER_USAGE_DYNAMIC_READ:
    case G_BUFFER_USAGE_STREAM_READ:    
    case G_BUFFER_USAGE_STATIC_READ:
        return 0; // No access

    default:
        report_invalid_enum(usage);
        return -1;
    }
}

s32 DirectX11::convert_draw_mode(graphics_enum_t draw_mode) {
    switch (draw_mode) {
        case G_DRAW_MODE_POINTS:                   return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        case G_DRAW_MODE_LINE_STRIP:               return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case G_DRAW_MODE_LINE_LOOP:                return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case G_DRAW_MODE_LINES:                    return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case G_DRAW_MODE_LINE_STRIP_ADJACENCY:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
        case G_DRAW_MODE_LINES_ADJACENCY:          return D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
        case G_DRAW_MODE_TRIANGLE_STRIP:           return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case G_DRAW_MODE_TRIANGLE_FAN:             return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case G_DRAW_MODE_TRIANGLES:                return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case G_DRAW_MODE_TRIANGLE_STRIP_ADJACENCY: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
        case G_DRAW_MODE_TRIANGLES_ADJACENCY:      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
        case G_DRAW_MODE_PATCHES:                  return D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;

        default:
            report_invalid_enum(draw_mode);
            return 0;
    }
}

s32 DirectX11::convert_data_type_to_shader_type(graphics_enum_t data_type) {
    switch (data_type)
    {
        case G_DATA_TYPE_U8:      return DXGI_FORMAT_R8_UINT;
        case G_DATA_TYPE_S8:      return DXGI_FORMAT_R8_SINT;
        case G_DATA_TYPE_U8VEC2:  return DXGI_FORMAT_R8G8_UINT;
        case G_DATA_TYPE_S8VEC2:  return DXGI_FORMAT_R8G8_SINT;
        case G_DATA_TYPE_U8VEC3:  return DXGI_FORMAT_R8G8B8A8_UINT;
        case G_DATA_TYPE_S8VEC3:  return DXGI_FORMAT_R8G8B8A8_SINT;
        case G_DATA_TYPE_U8VEC4:  return DXGI_FORMAT_R8G8B8A8_UINT;
        case G_DATA_TYPE_S8VEC4:  return DXGI_FORMAT_R8G8B8A8_SINT;
        case G_DATA_TYPE_U16:     return DXGI_FORMAT_R16_UINT;
        case G_DATA_TYPE_S16:     return DXGI_FORMAT_R16_SINT;
        case G_DATA_TYPE_U16VEC2: return DXGI_FORMAT_R16G16_UINT;
        case G_DATA_TYPE_S16VEC2: return DXGI_FORMAT_R16G16_SINT;
        case G_DATA_TYPE_U16VEC3: return DXGI_FORMAT_R16G16B16A16_UINT;
        case G_DATA_TYPE_S16VEC3: return DXGI_FORMAT_R16G16B16A16_SINT;
        case G_DATA_TYPE_U16VEC4: return DXGI_FORMAT_R16G16B16A16_UINT;
        case G_DATA_TYPE_S16VEC4: return DXGI_FORMAT_R16G16B16A16_SINT;
        case G_DATA_TYPE_U32:     return DXGI_FORMAT_R32_UINT;
        case G_DATA_TYPE_S32:     return DXGI_FORMAT_R32_SINT;
        case G_DATA_TYPE_U32VEC2: return DXGI_FORMAT_R32G32_UINT;
        case G_DATA_TYPE_S32VEC2: return DXGI_FORMAT_R32G32_SINT;
        case G_DATA_TYPE_U32VEC3: return DXGI_FORMAT_R32G32B32A32_UINT;
        case G_DATA_TYPE_S32VEC3: return DXGI_FORMAT_R32G32B32A32_SINT;
        case G_DATA_TYPE_U32VEC4: return DXGI_FORMAT_R32G32B32A32_UINT;
        case G_DATA_TYPE_S32VEC4: return DXGI_FORMAT_R32G32B32A32_SINT;
        case G_DATA_TYPE_F32:     return DXGI_FORMAT_R32_FLOAT;
        case G_DATA_TYPE_F32VEC2: return DXGI_FORMAT_R32G32_FLOAT;
        case G_DATA_TYPE_F32VEC3: return DXGI_FORMAT_R32G32B32_FLOAT;
        case G_DATA_TYPE_F32VEC4: return DXGI_FORMAT_R32G32B32A32_FLOAT;


        /*case G_DATA_TYPE_U8:
        case G_DATA_TYPE_U8VEC2:
        case G_DATA_TYPE_U8VEC3:
        case G_DATA_TYPE_U8VEC4:
            return DXGI_FORMAT_R8_UINT;

        case G_DATA_TYPE_S8:
        case G_DATA_TYPE_S8VEC2:
        case G_DATA_TYPE_S8VEC3:
        case G_DATA_TYPE_S8VEC4:
            return DXGI_FORMAT_R8_SINT;

        case G_DATA_TYPE_U16:
        case G_DATA_TYPE_U16VEC2:
        case G_DATA_TYPE_U16VEC3:
        case G_DATA_TYPE_U16VEC4:
            return DXGI_FORMAT_R16_UINT;

        case G_DATA_TYPE_S16VEC2:
        case G_DATA_TYPE_S16VEC3:
        case G_DATA_TYPE_S16VEC4:
        case G_DATA_TYPE_S16:
            return DXGI_FORMAT_R16_SINT;

        case G_DATA_TYPE_U32:
        case G_DATA_TYPE_U32VEC2:
        case G_DATA_TYPE_U32VEC3:
        case G_DATA_TYPE_U32VEC4:
            return DXGI_FORMAT_R32_UINT;

        case G_DATA_TYPE_S32VEC2:
        case G_DATA_TYPE_S32VEC3:
        case G_DATA_TYPE_S32VEC4:
        case G_DATA_TYPE_S32:
        case G_DATA_TYPE_TEXTURE:
            return DXGI_FORMAT_R32_SINT;

        case G_DATA_TYPE_F32:
        case G_DATA_TYPE_F32VEC3:
        case G_DATA_TYPE_F32VEC2:
        case G_DATA_TYPE_F32VEC4:
        case G_DATA_TYPE_F32MAT4:
            return DXGI_FORMAT_R32_FLOAT;*/
            
    default:
        report_invalid_enum(data_type);
        return 0;
    }
}

s32 DirectX11::convert_texture_format(graphics_enum_t fmt) {
    switch (fmt) {
    case G_TEXTURE_FORMAT_RED:          return DXGI_FORMAT_R32_FLOAT;
    case G_TEXTURE_FORMAT_RG:           return DXGI_FORMAT_R32G32_FLOAT;
    case G_TEXTURE_FORMAT_RGB:          return DXGI_FORMAT_R32G32B32_FLOAT;
    case G_TEXTURE_FORMAT_RGBA:         return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case G_TEXTURE_FORMAT_RED_INTEGER:  return DXGI_FORMAT_R8_UINT;
    case G_TEXTURE_FORMAT_RG_INTEGER:   return DXGI_FORMAT_R8G8_UINT;
    case G_TEXTURE_FORMAT_RGB_INTEGER:  return DXGI_FORMAT_R8G8B8A8_UINT;
    case G_TEXTURE_FORMAT_RGBA_INTEGER: return DXGI_FORMAT_R8G8B8A8_UINT;
    default:
        report_invalid_enum(fmt);
        return 0;
    }
}

size_t get_format_size(DXGI_FORMAT fmt) {
    switch (fmt) {
    case DXGI_FORMAT_R32_FLOAT:          return 4;
    case DXGI_FORMAT_R32G32_FLOAT:       return 8;
    case DXGI_FORMAT_R32G32B32_FLOAT:    return 12;
    case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
    case DXGI_FORMAT_R8_UINT:            return 1;
    case DXGI_FORMAT_R8G8_UINT:          return 2;
    case DXGI_FORMAT_R8G8B8A8_UINT:      return 4;

    default:
        _AP_BREAK;
        return 0;
    }
}

struct Window_Input_Context {
    bool down_flags[AP_LAST_KEY + 1];
    mz::ivec2 mouse_position;
    
    void OnKeyPress(input_code_t code) {
        down_flags[code] = true;
    }

    void OnKeyRelease(input_code_t code) {
        down_flags[code] = false;
    }
};

struct Vertex_Buffer_Data {
    ID3D11Buffer* dx11_buffer = NULL;

    // Deferred initialization data
    D3D11_BUFFER_DESC bd;
    void* data;
};

struct Index_Buffer_Data {
    ID3D11Buffer* dx11_buffer = NULL;
    u32* indices;
    count_t count;
};

struct Vertex_Array_Data {
    Dynamic_Array<graphics_id_t> vertex_buffers;
    graphics_id_t index_buffer = 0;
    Buffer_Layout_Specification layout;
    ID3D11InputLayout* dx11_layout = NULL;
};

struct Shader_Program_Data {
    graphics_id_t vertex_shader_handle = 0;
    graphics_id_t pixel_shader_handle = 0;
    Buffer_Layout_Specification input_layout;
};

struct Vertex_Shader_Data {
    ID3D11VertexShader* shader = NULL;
    ID3DBlob* blob = NULL;
};
struct Pixel_Shader_Data {
    ID3D11PixelShader* shader = NULL;
    ID3DBlob* blob = NULL;
};

struct Constant_Buffer_Data {
    ID3D11Buffer* dx11_buffer = NULL;
    void* data = NULL;
    Buffer_Layout_Specification layout;
};

struct Texture_Data {
    ID3D11Texture2D* dx11_texture = NULL;
    ID3D11ShaderResourceView* dx11_view = NULL;
    D3D11_TEXTURE2D_DESC td = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
};

struct DX11_Data {
    HINSTANCE instance_handle;
    ID3D11Device* device;
    IDXGISwapChain* swap_chain;
    ID3D11DeviceContext* device_context;
    ID3D11RenderTargetView* render_target;
    ID3D11SamplerState* sampler_state;

    mz::color16 clear_color = { 0.f, 0.f, 0.f, 1.f };
    Dynamic_Array<Vertex_Buffer_Data> vertex_buffers;
    Dynamic_Array<Index_Buffer_Data> index_buffers;
    Dynamic_Array<Vertex_Array_Data> vertex_arrays;

    Dynamic_Array<Shader_Program_Data> shader_programs;
    Dynamic_Array<Vertex_Shader_Data> vertex_shaders;
    Dynamic_Array<Pixel_Shader_Data> pixel_shaders;
    Dynamic_Array<Constant_Buffer_Data> constant_buffers;
    Dynamic_Array<Texture_Data> textures;

    Static_Array<graphics_id_t, 32> texture_bindings;
};

Hash_Map<void*, bool> window_close_flags;
Hash_Map<void*, Window_Input_Context> window_input_contexts;

LRESULT CALLBACK WndProc (HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CLOSE: 
            PostQuitMessage(0); 
            window_close_flags[window_handle] = true;
            break;
        case WM_KEYDOWN:
            window_input_contexts[window_handle].OnKeyPress(convert_key_code((int)wparam));
            break;
        case WM_KEYUP:
            window_input_contexts[window_handle].OnKeyRelease(convert_key_code((int)wparam));
            break;
    }
    return DefWindowProc(window_handle, message, wparam, lparam);
}

void DirectX11::init(bool show_window) {
    d = new DX11_Data();

    d->texture_bindings.fill(G_NULL_ID);

    d->instance_handle = GetModuleHandle(NULL);
    
    const auto class_name = L"ApparatusWindow";

    send_message(G_DEBUG_MESSAGE_SEVERITY_NOTIFY, "Creating a WinApi window...");

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = d->instance_handle;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszClassName = class_name;
    wc.hIconSm = NULL;

    RegisterClassEx(&wc);
    
    RECT window_rect;
    window_rect.left = 0;
    window_rect.top = 0;
    window_rect.right = 1280;
    window_rect.bottom = 720;
    AdjustWindowRectEx(&window_rect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE, 0);

    _windows_context.main_window_handle = CreateWindowEx(
        0,
        class_name,
        L"Apparatus Window",
        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
        NULL,
        NULL,
        d->instance_handle,
        this
    );

    auto main_window = _windows_context.main_window_handle;

    window_close_flags[main_window] = false;
    memset(window_input_contexts[main_window].down_flags, 0, sizeof(window_input_contexts[main_window].down_flags));

    if (show_window)
        ShowWindow((HWND)main_window, SW_SHOW);

    POINT pos;
    ScreenToClient((HWND)main_window, &pos);

    RECT area;
    GetClientRect((HWND)main_window, &area);

    auto& info = _windows_context.window_info[main_window];
    info.pos = { pos.x, pos.y };
    info.size = { area.right - area.left, area.bottom - area.top };
    info.title = "Apparatus Window";

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = (HWND)_windows_context.main_window_handle;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    auto create_device_flags = 0;
#ifdef _CONFIG_DEBUG
    create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    check_result(D3D11CreateDeviceAndSwapChain, "Failed creating device and swap chain.",
                 NULL,
                 D3D_DRIVER_TYPE_HARDWARE,
                 NULL,
                 create_device_flags,
                 NULL,
                 0,
                 D3D11_SDK_VERSION,
                 &sd,
                 &d->swap_chain,
                 &d->device,
                 NULL,
                 &d->device_context);
    
    ID3D11Resource* back_buffer = NULL;
    check_result(d->swap_chain->GetBuffer, "Failed getting back buffer in swap chain",
                 0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&back_buffer));
    check_result(d->device->CreateRenderTargetView,
                 "Failed creating render target view from swap chain back buffer.",
                 back_buffer, NULL, &d->render_target);

    back_buffer->Release();

    D3D11_SAMPLER_DESC smd = {};
    smd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    smd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    smd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    smd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    check_result(d->device->CreateSamplerState, "Failed creating sampler state", 
                 &smd, &d->sampler_state);

    _windows_context.main_window_handle = main_window;
    _windows_context.__should_close = [&](void* wnd) { 
        (void)wnd;
        return window_close_flags[wnd]; 
    };
    _windows_context.__swap_buffers = [&](void* wnd) { 
        (void)wnd;

        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        POINT pos;
        if (GetCursorPos(&pos))
        {
            ScreenToClient((HWND)_windows_context.main_window_handle, &pos);
            window_input_contexts[(HWND)wnd].mouse_position = { pos.x, pos.y };
        }

        check_result(d->swap_chain->Present, "Failed presenting", 1u, 0u);
    };
    _windows_context.__is_key_down = [](void* wnd, input_code_t k) {
        if (k <= 0 || k > AP_LAST_KEY) return false;
        return window_input_contexts[wnd].down_flags[k];
    };
    _windows_context.__get_mouse_position = [](void* wnd) {
        return window_input_contexts[wnd].mouse_position;
    };
}

DirectX11::~DirectX11() {
    if (d->render_target) d->render_target->Release();
    if (d->device) d->device->Release();
    if (d->device_context) d->device_context->Release();
    if (d->swap_chain) d->swap_chain->Release();

    DestroyWindow((HWND)_windows_context.main_window_handle);
    for (auto sub_window : _windows_context.window_handles) {
        DestroyWindow((HWND)sub_window);
    }
}

void DirectX11::set_culling(graphics_enum_t value) {
    (void)value;
}
void DirectX11::set_blending(bool value) {
    (void)value;
    //d->device_context->OMSetBlendState()
}

graphics_id_t DirectX11::make_vertex_array(const Buffer_Layout_Specification& layout) {
    graphics_id_t va = (graphics_id_t)d->vertex_arrays.size();

    d->vertex_arrays.emplace_back();

    auto& vb_data = d->vertex_arrays[va];
    vb_data.layout = layout;

    return va;
}
graphics_id_t DirectX11::make_shader_source(graphics_enum_t source_type, const char* src) {
    graphics_id_t shader_source = 0;

    auto ver = "";

    if (source_type == G_SHADER_SOURCE_TYPE_VERTEX) {
        ver = "vs_5_0";
        shader_source = (graphics_id_t)d->vertex_shaders.size();
        d->vertex_shaders.emplace_back();
    } else if (source_type == G_SHADER_SOURCE_TYPE_FRAGMENT) {
        ver = "ps_5_0";
        shader_source = (graphics_id_t)d->pixel_shaders.size();
        d->pixel_shaders.emplace_back();
    }

    ID3DBlob* compiled;
    ID3DBlob* error_message;
    HRESULT hr = D3DCompile(
        src, 
        strlen(src), 
        NULL, 
        NULL, 
        NULL,
        "main",
        ver,
        D3DCOMPILE_ENABLE_STRICTNESS,
        0,
        &compiled,
        &error_message
    );

    if (FAILED(hr)) {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        std::wstring wstr = errMsg;
        std::string str = std::filesystem::path(wstr).generic_string();

        send_message(G_DEBUG_MESSAGE_SEVERITY_ERROR, "Failed compiling shader (%s)\nError message: %s\nCompile errors:\n%s", 
                     get_graphics_enum_string(source_type), str.c_str(), (char*)error_message->GetBufferPointer());
        return 0;
    }

    if (source_type == G_SHADER_SOURCE_TYPE_VERTEX) {
        d->vertex_shaders[shader_source].blob = compiled;
        check_result(d->device->CreateVertexShader, "Failed creating vertex shader",
                     compiled->GetBufferPointer(), 
                     compiled->GetBufferSize(),
                     NULL,
                     &d->vertex_shaders[shader_source].shader
        );
    } else if (source_type == G_SHADER_SOURCE_TYPE_FRAGMENT) {
        d->pixel_shaders[shader_source].blob = compiled;
        check_result(d->device->CreatePixelShader, "Failed creating pixel shader",
                     compiled->GetBufferPointer(), 
                     compiled->GetBufferSize(),
                     NULL,
                     &d->pixel_shaders[shader_source].shader
        );
    }

    return shader_source;
}
graphics_id_t DirectX11::make_shader(graphics_id_t vert_shader, graphics_id_t frag_shader, const Buffer_Layout_Specification& input_layout) {
    graphics_id_t shader = (graphics_id_t)d->shader_programs.size();
    d->shader_programs.push_back({ vert_shader, frag_shader });
    d->shader_programs[shader].input_layout = input_layout;
    return shader;
}
graphics_id_t DirectX11::make_vertex_buffer(void* data, size_t sz, graphics_enum_t usage) {
    graphics_id_t vb = (graphics_id_t)d->vertex_buffers.size();

    d->vertex_buffers.emplace_back();

    auto& vb_data = d->vertex_buffers[vb];

    vb_data.bd = {};
    vb_data.bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vb_data.bd.ByteWidth = (UINT)sz;
    vb_data.bd.CPUAccessFlags = convert_buffer_usage_cpu_access(usage);
    vb_data.bd.MiscFlags = 0;
    vb_data.bd.Usage = (D3D11_USAGE)convert_buffer_usage(usage);

    vb_data.data = data;

    return vb;
}
graphics_id_t DirectX11::make_index_buffer(u32* indices, count_t count, graphics_enum_t usage) {
    graphics_id_t ib = (graphics_id_t)d->index_buffers.size();

    d->index_buffers.emplace_back();

    auto& ib_data = d->index_buffers[ib];

    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = (UINT)(count * sizeof(u32));
    bd.CPUAccessFlags = convert_buffer_usage_cpu_access(usage);
    bd.MiscFlags = 0;
    bd.Usage = (D3D11_USAGE)convert_buffer_usage(usage);
    bd.StructureByteStride = sizeof(u32);

    ib_data.indices = indices;
    ib_data.count = count;

    D3D11_SUBRESOURCE_DATA dx11_data = {};
    dx11_data.pSysMem = indices;
    check_result(d->device->CreateBuffer, "Failed creating index buffer", 
                 &bd, &dx11_data, &ib_data.dx11_buffer);

    return ib;
}
graphics_id_t DirectX11::make_uniform_buffer(void* data, const Buffer_Layout_Specification& layout, graphics_enum_t usage) {
    graphics_id_t ub = (graphics_id_t)d->constant_buffers.size();

    d->constant_buffers.emplace_back();

    auto& ub_data = d->constant_buffers[ub];

    ub_data.data = data;
    ub_data.layout = layout;
    ub_data.layout.align_each_entry(16);

    D3D11_BUFFER_DESC bd;
    bd.ByteWidth = (UINT)ub_data.layout.stride;
    bd.Usage = (D3D11_USAGE)convert_buffer_usage(usage);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = (UINT)convert_buffer_usage_cpu_access(usage);
    bd.MiscFlags = 0u;
    bd.StructureByteStride = 0u;
    D3D11_SUBRESOURCE_DATA sd;
    sd.pSysMem = ub_data.data;
    check_result(d->device->CreateBuffer, "Failed creating uniform buffer", 
                 &bd, &sd, &ub_data.dx11_buffer);

    return ub;
}
graphics_id_t DirectX11::make_texture(graphics_enum_t usage) {
    graphics_id_t tex = (graphics_id_t)d->textures.size();

    d->textures.emplace_back();

    auto& tex_data = d->textures[tex];

    tex_data.td = {};
    tex_data.srvd = {};
    tex_data.td.Usage = (D3D11_USAGE)convert_buffer_usage(usage);
    tex_data.td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    tex_data.td.CPUAccessFlags = (UINT)convert_buffer_usage_cpu_access(usage);
    tex_data.td.MiscFlags = 0u;

    tex_data.td.SampleDesc.Count = 1;
    tex_data.td.SampleDesc.Quality = 0;

    return tex;
}

void DirectX11::destroy_vertex_array(graphics_id_t vao) {
    (void)vao;
    // TODO: Should mark vao index as free
}
void DirectX11::destroy_shader(graphics_id_t shader) {
    (void)shader;
    // TODO: Should mark shader index as free
}
void DirectX11::destroy_vertex_buffer(graphics_id_t vbo) {
    auto& vb_data = d->vertex_buffers[vbo];

    vb_data.dx11_buffer->Release();
}
void DirectX11::destroy_index_buffer(graphics_id_t ibo) {
    auto& ib_data = d->index_buffers[ibo];

    ib_data.dx11_buffer->Release();
}
void DirectX11::destroy_uniform_buffer(graphics_id_t ubo) {
    auto& ub_data = d->constant_buffers[ubo];

    ub_data.dx11_buffer->Release();
}
void DirectX11::destroy_texture(graphics_id_t texture) {
    auto& tex_data = d->textures[texture];

    tex_data.dx11_texture->Release();
}

void DirectX11::set_texture_data(graphics_id_t texture, byte* data, mz::ivec2 size, graphics_enum_t fmt) {
    (void)texture; (void)data; (void)size; (void)fmt;

    auto& tex_data = d->textures[texture];

    if (tex_data.dx11_texture) {
        tex_data.dx11_texture->Release();
        tex_data.dx11_texture = NULL;
    }

    tex_data.td.Width = size.width;
    tex_data.td.Height = size.height;
    tex_data.td.MipLevels = 1;
    tex_data.td.Format = (DXGI_FORMAT)convert_texture_format(fmt);
    tex_data.td.ArraySize = 1;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = data;
    sd.SysMemPitch = size.width * (UINT)get_format_size(tex_data.td.Format);

    check_result(d->device->CreateTexture2D, "Failed creating texture", 
                 &tex_data.td, &sd, &tex_data.dx11_texture);

    tex_data.srvd.Format = tex_data.td.Format;
    tex_data.srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    tex_data.srvd.Texture2D.MostDetailedMip = 0;
    tex_data.srvd.Texture2D.MipLevels = tex_data.td.MipLevels;
    check_result(d->device->CreateShaderResourceView, "Failed creating texture view", 
        tex_data.dx11_texture, &tex_data.srvd, &tex_data.dx11_view);
}
void DirectX11::set_texture_wrapping(graphics_id_t texture, graphics_enum_t wrap_mode) {
    (void)texture; (void)wrap_mode;
}
void DirectX11::set_texture_filtering(graphics_id_t texture, graphics_enum_t min_filter_mode, graphics_enum_t mag_filter_mode) {
    (void)texture; (void)min_filter_mode; (void)mag_filter_mode;

    /*auto& tex_data = d->textures[texture];

    if (min_filter_mode == G_MIN_FILTER_LINEAR_MIPMAP_LINEAR
     || min_filter_mode == G_MIN_FILTER_LINEAR_MIPMAP_NEAREST
     || min_filter_mode == G_MIN_FILTER_NEAREST_MIPMAP_LINEAR
     || min_filter_mode == G_MIN_FILTER_NEAREST_MIPMAP_NEAREST) {
        tex_data.td.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }*/
}
void DirectX11::set_texture_multisampling(graphics_id_t texture, u32 count, u32 quality) {
    auto& tex_data = d->textures[texture];

    DXGI_SAMPLE_DESC sd;
    sd.Count = count;
    sd.Quality = quality;
    
    tex_data.td.SampleDesc = sd;
}

void DirectX11::bind_texture_to_slot(graphics_id_t texture, graphics_enum_t slot) {
    d->texture_bindings[slot] = texture;
}

void DirectX11::set_clear_color(mz::color16 color) {
    d->clear_color = color;
}

void DirectX11::set_viewport(mz::viewport viewport) {
    D3D11_VIEWPORT dx11_viewport;
    dx11_viewport.TopLeftX = (FLOAT)viewport.x;
    dx11_viewport.TopLeftY = (FLOAT)viewport.y;
    dx11_viewport.Width = (FLOAT)viewport.width;
    dx11_viewport.Height = (FLOAT)viewport.height;
    dx11_viewport.MaxDepth = 1.0f;
    dx11_viewport.MinDepth = 0.0f;
    d->device_context->RSSetViewports(1, &dx11_viewport);
}

void DirectX11::clear(graphics_enum_t clear_flags) {
    (void)clear_flags;
    d->device_context->ClearRenderTargetView(d->render_target, d->clear_color.ptr);
}

void DirectX11::draw_indices(graphics_id_t vao, graphics_id_t shader, graphics_id_t ubo, graphics_enum_t draw_mode) {
    static Dynamic_Array<ID3D11Buffer*> dx11_vertex_buffers;
    static Dynamic_Array<UINT> dx11_strides;
    static Dynamic_Array<UINT> dx11_offsets;

    auto& va_data = d->vertex_arrays[vao];
    auto& layout = va_data.layout;
    auto& program = d->shader_programs[shader];
    auto& vertex_shader_data = d->vertex_shaders[program.vertex_shader_handle];
    auto& pixel_shader_data = d->pixel_shaders[program.pixel_shader_handle];

    if (!program.input_layout.is_sated_by(va_data.layout)) {
        send_message(G_DEBUG_MESSAGE_SEVERITY_CRITICAL, "Vao layout is not compatible with shader input layout. Shader input layout needs to be sated.");
    }

    if (!va_data.dx11_layout) {
        Dynamic_Array<D3D11_INPUT_ELEMENT_DESC> dx11_elements;

        for (int location = 0; location < layout.entries.size(); location++) {
            auto& entry = layout.entries[location];

            D3D11_INPUT_ELEMENT_DESC element;
            element.SemanticName = entry.name;
            element.SemanticIndex = 0;
            element.Format = (DXGI_FORMAT)convert_data_type_to_shader_type(entry.data_type);
            element.InputSlot = 0; 
            element.AlignedByteOffset = (UINT)entry.offset;
            element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            element.InstanceDataStepRate = 0;

            dx11_elements.push_back(element); 
        }                                     
        check_result(d->device->CreateInputLayout, "Failed creating input layout",
                dx11_elements.data(),
                (UINT)dx11_elements.size(),
                vertex_shader_data.blob->GetBufferPointer(), 
                vertex_shader_data.blob->GetBufferSize(),
                &va_data.dx11_layout);
    }

    for (s32 i = 0; i < va_data.vertex_buffers.size(); i++) {
        auto& vb_data = d->vertex_buffers[va_data.vertex_buffers[i]];

        if (!vb_data.dx11_buffer) {
            vb_data.bd.StructureByteStride = (UINT)layout.stride;
        
            D3D11_SUBRESOURCE_DATA dx11_data = {};
            dx11_data.pSysMem = vb_data.data;
            check_result(d->device->CreateBuffer, "Failed creating vertex buffer", 
                        &vb_data.bd, &dx11_data, &vb_data.dx11_buffer);
        }

        dx11_vertex_buffers.push_back(vb_data.dx11_buffer);
        dx11_strides.push_back((UINT)layout.stride);
        dx11_offsets.push_back(0u);
    }

    d->device_context->IASetVertexBuffers(
        0u, 
        (UINT)dx11_vertex_buffers.size(), 
        dx11_vertex_buffers.data(),
        dx11_strides.data(),
        dx11_offsets.data()
    );

    auto& ib_data = d->index_buffers[va_data.index_buffer];

    d->device_context->IASetIndexBuffer(ib_data.dx11_buffer, DXGI_FORMAT_R32_UINT, 0u);
    d->device_context->IASetInputLayout(va_data.dx11_layout);
    d->device_context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)convert_draw_mode(draw_mode));
    
    d->device_context->VSSetShader(vertex_shader_data.shader, 0, 0);
    d->device_context->PSSetShader(pixel_shader_data.shader, 0, 0);

    d->device_context->OMSetRenderTargets(1, &d->render_target, NULL);

    if (ubo != G_NULL_ID) {
        auto& ub_data = d->constant_buffers[ubo];

        d->device_context->VSSetConstantBuffers(0, 1, &ub_data.dx11_buffer);
    } else {
        d->device_context->VSSetConstantBuffers(0, 1, NULL);
    }

    static Static_Array<ID3D11ShaderResourceView*, 32> to_upload;

    for (int i = 0; i < 32; i++) {
        if (d->texture_bindings[i] != G_NULL_ID) {
            auto& tex_data = d->textures[d->texture_bindings[i]];
            to_upload[i] = tex_data.dx11_view;
        } else {
            to_upload[i] = NULL;
        }
    }

    d->device_context->PSSetShaderResources(0, 32, to_upload.data());

    d->device_context->PSSetSamplers(0u, 1u, &d->sampler_state);

    d->device_context->DrawIndexed((UINT)ib_data.count, 0u, 0);

    dx11_vertex_buffers.clear();
    dx11_strides.clear();
    dx11_offsets.clear();
}

void DirectX11::associate_vertex_buffer(graphics_id_t vbo, graphics_id_t vao) {
    auto& va_data = d->vertex_arrays[vao];

    va_data.vertex_buffers.push_back(vbo);
}
void DirectX11::associate_index_buffer(graphics_id_t ibo, graphics_id_t vao) {
    auto& va_data = d->vertex_arrays[vao];

    va_data.index_buffer = ibo;
}

void DirectX11::set_vertex_buffer_data(graphics_id_t vbo, void* data, size_t offset, size_t sz) {
    (void)vbo; (void)data; (void)offset; (void)sz;
}
void DirectX11::set_index_buffer_data(graphics_id_t ibo, u32* indices, count_t offset, count_t count) {
    (void)ibo; (void)indices; (void)offset; (void)count;
}

void DirectX11::set_uniform_buffer_data(graphics_id_t ubo, const char* name, void* data) {

    auto& ub_data = d->constant_buffers[ubo];
    ub_data.data = data;

    for (auto& entry : ub_data.layout.entries) {
        if (strcmp(entry.name, name) == 0) {
            D3D11_MAPPED_SUBRESOURCE ref;
            check_result(d->device_context->Map, "Failed mapping constant buffer", 
                ub_data.dx11_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ref);
            ap_assert(entry.offset + entry.size <= ub_data.layout.stride, "Ubo buffer too small");
            memcpy((byte*)ref.pData + entry.offset, data, entry.size);
            d->device_context->Unmap(ub_data.dx11_buffer, 0);

            break;
        }
    }
}

void* DirectX11::map_vertex_buffer_data(graphics_id_t vbo) {
    auto& vb_data = d->vertex_buffers[vbo];

    D3D11_MAPPED_SUBRESOURCE ref;
    check_result(d->device_context->Map, "Failed mapping vertex buffer", 
        vb_data.dx11_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ref);
    
    return ref.pData;
}
u32* DirectX11::map_index_buffer_data(graphics_id_t ibo) {
    auto& ib_data = d->index_buffers[ibo];

    D3D11_MAPPED_SUBRESOURCE ref;
    check_result(d->device_context->Map, "Failed mapping index buffer", 
        ib_data.dx11_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ref);

    return (u32*)ref.pData;
}
void* DirectX11::map_uniform_buffer_data(graphics_id_t ubo) {
    auto& ub_data = d->constant_buffers[ubo];

    D3D11_MAPPED_SUBRESOURCE ref;
    check_result(d->device_context->Map, "Failed mapping uniform buffer", 
        ub_data.dx11_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ref);

    return ref.pData;
}
void DirectX11::unmap_vertex_buffer_data(graphics_id_t vbo) {
    auto& vb_data = d->vertex_buffers[vbo];

    d->device_context->Unmap(vb_data.dx11_buffer, 0);
}
void DirectX11::unmap_index_buffer_data(graphics_id_t ibo) {
    auto& ib_data = d->index_buffers[ibo];

    d->device_context->Unmap(ib_data.dx11_buffer, 0);
}
void DirectX11::unmap_uniform_buffer_data(graphics_id_t ubo) {
    auto& ub_data = d->constant_buffers[ubo];

    d->device_context->Unmap(ub_data.dx11_buffer, 0);
}

#endif // AP_SUPPORT_DX11