#pragma once

#include "graphics/graphics_api.h"

#ifdef AP_SUPPORT_OPENGL45

struct OpenGL45 {
    void init(bool show_window);
    ~OpenGL45();

    _impl_api;

    inline Windows_Context* get_windows_context() { return &_windows_context; }

    u32 convert_shader_source_type(graphics_enum_t shader_source_type);
    u32 convert_buffer_usage(graphics_enum_t usage);
    u32 convert_data_type_to_shader_type(graphics_enum_t data_type);
    u32 convert_draw_mode(graphics_enum_t draw_mode);
    u32 convert_texture_wrapping(graphics_enum_t value);
    u32 convert_texture_filtering(graphics_enum_t value);
    u32 convert_texture_format(graphics_enum_t fmt);;
    u32 convert_texture_slot(graphics_enum_t slot);

    Dynamic_Array<graphics_id_t> _ibo_associations;
    Dynamic_Array<count_t> _index_count;
    Dynamic_Array<Buffer_Layout_Specification> _buffer_layouts;
    Dynamic_Array<Buffer_Layout_Specification> _shader_input_layouts;
    Dynamic_Array<Buffer_Layout_Specification> _uniform_buffer_layouts;

    Graphics<OpenGL45>* _api;

    Windows_Context _windows_context;
    ImGuiContext* _imgui_context;
};

#endif // AP_SUPPORT_OPENGL45