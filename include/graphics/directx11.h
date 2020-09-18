#pragma once

#include "graphics/graphics_api.h"

#ifdef AP_SUPPORT_DX11

struct DX11_Data;

struct DirectX11 {
    void init(bool show_window);

    ~DirectX11();

    _impl_api;

    inline Windows_Context* get_windows_context() { return &_windows_context; }

    s32 convert_buffer_usage(graphics_enum_t usage);
    s32 convert_buffer_usage_cpu_access(graphics_enum_t usage);
    s32 convert_draw_mode(graphics_enum_t draw_mode);
    s32 convert_data_type_to_shader_type(graphics_enum_t data_type);
    s32 convert_texture_format(graphics_enum_t fmt);

    Graphics_Context* _api;

    Windows_Context _windows_context;

    DX11_Data* d;
};

#endif // AP_SUPPORT_DX11