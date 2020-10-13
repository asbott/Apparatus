#pragma once

#define IM_VEC2_CLASS_EXTRA                                                 \
        template <typename value_t>\
        ImVec2(const mz::vec2<value_t>& f) { x = (f32)f.x; y = (f32)f.y; }                       \
        template <typename value_t>\
        operator mz::vec2<value_t>() const { return mz::vec2<value_t>((value_t)x,(value_t)y); }

#define IM_VEC4_CLASS_EXTRA                                                 \
        template <typename value_t>\
        ImVec4(const mz::vec4<value_t>& f) { x = (f32)f.x; y = (f32)f.y; z = (f32)f.z; w = (f32)f.w; }     \
        template <typename value_t>\
        operator mz::vec4<value_t>() const { return mz::vec4<value_t>((value_t)x,(value_t)y,(value_t)z,(value_t)w); }

#include <imgui.h>

#include "graphics_enum.h"

#include "input_codes.h"

#include "thread_server.h"

typedef void(*GraphicsDebugCallback)(graphics_enum_t severity, const char* msg, const char* fn);

typedef u32 graphics_id_t;

#define NOMINMAX
#undef max
constexpr graphics_id_t G_NULL_ID = std::numeric_limits<graphics_id_t>::max();

struct Input_State {
	bool keys_down[AP_KEY_COUNT];
	bool keys_press[AP_KEY_COUNT];

	bool mouse_down[AP_MOUSE_BUTTON_COUNT];
	bool mouse_press[AP_MOUSE_BUTTON_COUNT];

	mz::fvec2 mouse_pos;
};

struct Window_Info {
    Window_Info() {}
    Window_Info(const Window_Info& src)
        : size(src.size), pos(src.pos), title(src.title), delta_time(src.delta_time),
          _last_time(src._last_time), is_vsync_on(src.is_vsync_on), _input(src._input)  { }

    Window_Info& operator =(const Window_Info& src) {
        size = src.size;
        pos = src.pos;
        title = src.title;
        delta_time = src.delta_time;
        _last_time = src._last_time;
        is_vsync_on = src.is_vsync_on;
        _input = src._input;
        return *this;
    }

    mz::ivec2 size;
    mz::ivec2 pos;
    const char* title;
    f64 delta_time = 0;
    f64 _last_time = 0;
    bool is_vsync_on;
    Input_State _input;

    std::mutex _input_mutex;
};

struct Windows_Context {
    void* main_window_handle;
    Dynamic_Array<void*> window_handles;

    Hash_Map<void*, Window_Info> window_info; 

    std::function<bool(void* wnd)> __should_close;
    _ap_force_inline bool should_close(void* wnd) { return __should_close(wnd); }

    std::function<void(void* wnd)> __swap_buffers;
    _ap_force_inline void swap_buffers(void* wnd) { __swap_buffers(wnd); }

    std::function<bool(void* wnd, input_code_t k)> __is_key_down;
    _ap_force_inline bool is_key_down(void* wnd, input_code_t k) { return __is_key_down(wnd, k); }
    std::function<bool(void* wnd, input_code_t k)> __is_key_pressed;
    _ap_force_inline bool is_key_pressed(void* wnd, input_code_t k) { return __is_key_pressed(wnd, k); }

    std::function<mz::ivec2(void* wnd)> __get_mouse_position;
    _ap_force_inline mz::ivec2 get_mouse_position(void* wnd) { return __get_mouse_position(wnd); }

    std::function<void(void* wnd, bool value)> __set_vsync;
    _ap_force_inline void set_vsync(void* wnd, bool value) { __set_vsync(wnd, value); window_info[wnd].is_vsync_on = value; }

    std::function<void(void* wnd)> __hide_window;
    _ap_force_inline void hide_window(void* wnd) { __hide_window(wnd); }

    std::function<void(void* wnd)> __show_window;
    _ap_force_inline void show_window(void* wnd) { __show_window(wnd); }

    std::function<bool(void* wnd, input_code_t m)> __is_mouse_down;
    _ap_force_inline bool is_mouse_down(void* wnd, input_code_t m) { return __is_mouse_down(wnd, m); }
    std::function<bool(void* wnd, input_code_t m)> __is_mouse_pressed;
    _ap_force_inline bool is_mouse_pressed(void* wnd, input_code_t m) { return __is_mouse_pressed(wnd, m); }
};

struct Buffer_Layout_Specification {
    struct Buffer_Layout_Entry {
        Buffer_Layout_Entry(const char* _name, graphics_enum_t _data_type, bool _normalized = false)
            : name(_name) {
            data_type = _data_type;
            offset = 0;
            size = get_graphics_data_type_size(data_type);
            component_count = get_graphics_data_type_component_count(data_type);
            normalized = _normalized;
        }
        size_t offset;
        size_t size;
        count_t component_count;
        graphics_enum_t data_type;
        bool normalized;
        const char* name;
    };
    inline Buffer_Layout_Specification() {}
    inline Buffer_Layout_Specification(const std::initializer_list<Buffer_Layout_Entry>& _entries) {
        entries = _entries;
        for (auto& entry : entries) {
            entry.offset = stride;
            stride += entry.size;
        }
    }
    inline Buffer_Layout_Specification(const Buffer_Layout_Specification& rhs) {
        stride = rhs.stride;
        entries = rhs.entries;
    }

    Buffer_Layout_Specification& operator= (const Buffer_Layout_Specification& rhs) {
        stride = rhs.stride;
        entries = rhs.entries;
        return *this;
    }

    size_t stride = 0;
    Dynamic_Array<Buffer_Layout_Entry> entries;

    inline bool is_sated_by(const Buffer_Layout_Specification& other) const {
        if (other.entries.size() < entries.size()) return false;
        for (size_t i = 0; i < entries.size(); i++) {
            if (entries.at(i).size != other.entries.at(i).size) return false;
        }
        return true;
    }

    inline void align_each_entry(size_t alignment) {
        s64 extra_offset = 0;
        for (auto& entry : entries) {
            entry.offset += extra_offset;
            size_t remainder = entry.size % alignment;
            size_t nsize = remainder == 0 ? entry.size : entry.size + (alignment - remainder);
            s64 size_diff = nsize - entry.size;
            entry.size = nsize;
            extra_offset += size_diff;
            stride += size_diff;
        }
    }
};

struct AP_API Graphics_Context {

    virtual ~Graphics_Context() {}

    virtual void init(bool show_window = true) = 0;
    virtual void init(bool show_window, Thread_Server* thread_server, thread_id_t tid) = 0;

    virtual void set_culling(graphics_enum_t value) = 0;
    virtual void set_blending(bool value) = 0;

    virtual graphics_id_t make_vertex_array(const Buffer_Layout_Specification& layout) = 0;
    virtual graphics_id_t make_shader_source(graphics_enum_t source_type, const char* src) = 0;
    virtual graphics_id_t make_shader(graphics_id_t vert_shader, graphics_id_t frag_shader, const Buffer_Layout_Specification& input_layout) = 0;
    virtual graphics_id_t make_vertex_buffer(void* data, size_t sz, graphics_enum_t usage) = 0;
    virtual graphics_id_t make_index_buffer(u32* indices, count_t count, graphics_enum_t usage) = 0;
    virtual graphics_id_t make_uniform_buffer(void* data, const Buffer_Layout_Specification& layout, graphics_enum_t usage) = 0;
    virtual graphics_id_t make_texture(graphics_enum_t usage) = 0;
    virtual graphics_id_t make_render_target(mz::ivec2 size) = 0;

    virtual void destroy_vertex_array(graphics_id_t vao) = 0;
    virtual void destroy_shader(graphics_id_t shader) = 0;
    virtual void destroy_shader_source(graphics_id_t shader_source) = 0;
    virtual void destroy_vertex_buffer(graphics_id_t vbo) = 0;
    virtual void destroy_index_buffer(graphics_id_t ibo) = 0;
    virtual void destroy_uniform_buffer(graphics_id_t ubo) = 0;
    virtual void destroy_texture(graphics_id_t texture) = 0;
    virtual void destroy_render_target(graphics_id_t render_target) = 0;

    virtual void set_texture_data(graphics_id_t texture, byte* data, mz::ivec2 size, graphics_enum_t fmt) = 0;
    virtual void set_texture_wrapping(graphics_id_t texture, graphics_enum_t wrap_mode) = 0;
    virtual void set_texture_filtering(graphics_id_t texture, graphics_enum_t min_filter_mode, graphics_enum_t mag_filter_mode) = 0;
    virtual void set_texture_multisampling(graphics_id_t texture, u32 count, u32 quality) = 0;
    virtual void bind_texture_to_slot(graphics_id_t texture, graphics_enum_t slot) = 0;
    virtual void* get_native_texture_handle(graphics_id_t texture) = 0;

    virtual void set_render_target_size(graphics_id_t render_target, mz::ivec2 size) = 0;
    virtual mz::ivec2 get_render_target_size(graphics_id_t render_target) = 0;
    virtual graphics_id_t get_render_target_texture(graphics_id_t render_target) = 0;

    virtual void set_clear_color(mz::color16 color, graphics_id_t render_target = G_NULL_ID) = 0;

    virtual void set_viewport(mz::viewport viewport) = 0;

    virtual void clear(graphics_enum_t clear_flags, graphics_id_t render_target = G_NULL_ID) = 0;

    virtual void draw_indices(graphics_id_t vao, graphics_id_t shader, u32 index_count, graphics_id_t ubo = G_NULL_ID, graphics_enum_t draw_mode = G_DRAW_MODE_TRIANGLES, graphics_id_t render_target = G_NULL_ID) = 0;

    virtual void associate_vertex_buffer(graphics_id_t vbo, graphics_id_t vao) = 0;
    virtual void associate_index_buffer(graphics_id_t ibo, graphics_id_t vao) = 0;

    virtual void set_vertex_buffer_data(graphics_id_t vbo, void* data, size_t offset, size_t sz) = 0;
    virtual void set_index_buffer_data(graphics_id_t ibo, u32* indices, count_t offset, count_t count) = 0;
    virtual void set_uniform_buffer_data(graphics_id_t ubo, const char* name, void* data) = 0;

    virtual void* map_vertex_buffer_data(graphics_id_t vbo)  = 0;
    virtual u32* map_index_buffer_data(graphics_id_t ibo)   = 0;
    virtual void* map_uniform_buffer_data(graphics_id_t ubo) = 0;

    virtual void unmap_vertex_buffer_data(graphics_id_t vbo)  = 0;
    virtual void unmap_index_buffer_data(graphics_id_t ibo)   = 0;
    virtual void unmap_uniform_buffer_data(graphics_id_t ubo) = 0;

    virtual void use_imgui_context() = 0;

    virtual Windows_Context* get_windows_context() = 0;

    virtual void update_imgui() = 0;
    virtual void render_imgui() = 0;

    virtual void _send_debug_message(graphics_enum_t severity, const char* fn, const char* fmt, ...) = 0;

    GraphicsDebugCallback debug_callback = 0;
};

typedef std::function<void()> Graphics_Command;

template <typename api_t>
struct AP_API Graphics : public Graphics_Context {

    inline ~Graphics() {
         if (_should_delete_thread_server) delete _thread_server;
    }

    _ap_force_inline void init(bool show_window = true) override {
        _should_delete_thread_server = true;
        auto thread_server = new Thread_Server();
        init(show_window, thread_server, thread_server->make_thread(true));
    }

    _ap_force_inline void init(bool show_window, Thread_Server* thread_server, thread_id_t tid) override {
        _thread_server = thread_server;
        _tid = tid;

        _inst._api = this;

        _thread_server->queue_task(_tid, [this, show_window]() {_inst.init(show_window);} );
    }

    _ap_force_inline void set_culling(graphics_enum_t value) override {
        _thread_server->queue_task(_tid, [this, value]() { _inst.set_culling(value); });
    }
    _ap_force_inline void set_blending(bool value) override {
        _thread_server->queue_task(_tid, [this, value]() { _inst.set_blending(value); });
    }

    _ap_force_inline graphics_id_t make_vertex_array(const Buffer_Layout_Specification& layout) override {
        graphics_id_t ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, layout]() { ret = _inst.make_vertex_array(layout); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline graphics_id_t make_shader_source(graphics_enum_t source_type, const char* src) override {
        graphics_id_t ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, source_type, src]() { ret = _inst.make_shader_source(source_type, src); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline graphics_id_t make_shader(graphics_id_t vert_shader, graphics_id_t frag_shader, const Buffer_Layout_Specification& input_layout) override {
        graphics_id_t ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, vert_shader, frag_shader, input_layout]() { ret = _inst.make_shader(vert_shader, frag_shader, input_layout); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline graphics_id_t make_vertex_buffer(void* data, size_t sz, graphics_enum_t usage) override {
        graphics_id_t ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, data, sz, usage]() { ret = _inst.make_vertex_buffer(data, sz, usage); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline graphics_id_t make_index_buffer(u32* indices, count_t count, graphics_enum_t usage) override {
        graphics_id_t ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, indices, count, usage]() { ret = _inst.make_index_buffer(indices, count, usage); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline graphics_id_t make_uniform_buffer(void* data, const Buffer_Layout_Specification& layout, graphics_enum_t usage) override {
        graphics_id_t ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, data, layout, usage]() { ret = _inst.make_uniform_buffer(data, layout, usage); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline graphics_id_t make_texture(graphics_enum_t usage) override {
        graphics_id_t ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, usage]() { ret = _inst.make_texture(usage); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline graphics_id_t make_render_target(mz::ivec2 size) override {
        graphics_id_t ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, size]() { ret = _inst.make_render_target(size); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline void destroy_vertex_array(graphics_id_t vao) override {
        _thread_server->queue_task(_tid, [this, vao]() { _inst.destroy_vertex_array(vao); });
    }
    _ap_force_inline void destroy_shader(graphics_id_t shader) override {
        _thread_server->queue_task(_tid, [this, shader]() { _inst.destroy_shader(shader); });
    }
    _ap_force_inline void destroy_shader_source(graphics_id_t shader_source) override {
        _thread_server->queue_task(_tid, [this, shader_source]() { _inst.destroy_shader_source(shader_source); });
    }
    _ap_force_inline void destroy_vertex_buffer(graphics_id_t vbo) override {
        _thread_server->queue_task(_tid, [this, vbo]() { _inst.destroy_vertex_buffer(vbo); });
    }
    _ap_force_inline void destroy_index_buffer(graphics_id_t ibo) override {
        _thread_server->queue_task(_tid, [this, ibo]() { _inst.destroy_index_buffer(ibo); });
    }
    _ap_force_inline void destroy_uniform_buffer(graphics_id_t ubo) override {
        _thread_server->queue_task(_tid, [this, ubo]() { _inst.destroy_uniform_buffer(ubo); });
    }
    _ap_force_inline void destroy_texture(graphics_id_t texture) override {
        _thread_server->queue_task(_tid, [this, texture]() { _inst.destroy_texture(texture); });
    }
    _ap_force_inline void destroy_render_target(graphics_id_t render_target) override {
        _thread_server->queue_task(_tid, [this, render_target]() { _inst.destroy_render_target(render_target); });
    }

    _ap_force_inline void set_texture_data(graphics_id_t texture, byte* data, mz::ivec2 size, graphics_enum_t fmt) override {
        _thread_server->queue_task(_tid, [this, texture, data, size, fmt]() { _inst.set_texture_data(texture, data, size, fmt); });
    }
    _ap_force_inline void set_texture_wrapping(graphics_id_t texture, graphics_enum_t wrap_mode) override {
        _thread_server->queue_task(_tid, [this, texture, wrap_mode]() { _inst.set_texture_wrapping(texture, wrap_mode); });
    }
    _ap_force_inline void set_texture_filtering(graphics_id_t texture, graphics_enum_t min_filter_mode, graphics_enum_t mag_filter_mode) override {
        _thread_server->queue_task(_tid, [this, texture, min_filter_mode, mag_filter_mode]() { _inst.set_texture_filtering(texture, min_filter_mode, mag_filter_mode); });
    }
    _ap_force_inline void set_texture_multisampling(graphics_id_t texture, u32 count, u32 quality) override {
        _thread_server->queue_task(_tid, [this, texture, count, quality]() { _inst.set_texture_multisampling(texture, count, quality); });
    }

    _ap_force_inline void bind_texture_to_slot(graphics_id_t texture, graphics_enum_t slot) override {
        _thread_server->queue_task(_tid, [this, texture, slot]() { _inst.bind_texture_to_slot(texture, slot); });
    }

    _ap_force_inline void* get_native_texture_handle(graphics_id_t texture) override {
        void* ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, texture]() { ret = _inst.get_native_texture_handle(texture); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline void set_render_target_size(graphics_id_t render_target, mz::ivec2 size) {
        _thread_server->queue_task(_tid, [this, render_target, size]() { _inst.set_render_target_size(render_target, size); });
    }
    _ap_force_inline mz::ivec2 get_render_target_size(graphics_id_t render_target) override {
        mz::ivec2 ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, render_target]() { ret = _inst.get_render_target_size(render_target); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline graphics_id_t get_render_target_texture(graphics_id_t render_target) override {
        graphics_id_t ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, render_target]() { ret = _inst.get_render_target_texture(render_target); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline void set_clear_color(mz::color16 color, graphics_id_t render_target = G_NULL_ID) override {
        _thread_server->queue_task(_tid, [this, color, render_target]() { _inst.set_clear_color(color, render_target); });
    }

    _ap_force_inline void set_viewport(mz::viewport viewport) override {
        _thread_server->queue_task(_tid, [this, viewport]() { _inst.set_viewport(viewport); });
    }

    _ap_force_inline void clear(graphics_enum_t clear_flags, graphics_id_t render_target = G_NULL_ID) override {
        _thread_server->queue_task(_tid, [this, clear_flags, render_target]() { _inst.clear(clear_flags, render_target); });
    }

    _ap_force_inline void draw_indices(graphics_id_t vao, graphics_id_t shader, u32 index_count, graphics_id_t ubo, graphics_enum_t draw_mode = G_DRAW_MODE_TRIANGLES, graphics_id_t render_target = G_NULL_ID) override {
        _thread_server->queue_task(_tid, [this, vao, shader, index_count, ubo, draw_mode, render_target]() { _inst.draw_indices(vao, shader, index_count, ubo, draw_mode, render_target); });
    }

    _ap_force_inline void associate_vertex_buffer(graphics_id_t vbo, graphics_id_t vao) override {
        _thread_server->queue_task(_tid, [this, vbo, vao]() { _inst.associate_vertex_buffer(vbo, vao); });
    }
    _ap_force_inline void associate_index_buffer(graphics_id_t ibo, graphics_id_t vao) override {
        _thread_server->queue_task(_tid, [this, ibo, vao]() { _inst.associate_index_buffer(ibo, vao); });
    }

    _ap_force_inline void set_vertex_buffer_data(graphics_id_t vbo, void* data, size_t offset, size_t sz) override {
        _thread_server->queue_task(_tid, [this, vbo, data, offset, sz]() { _inst.set_vertex_buffer_data(vbo, data, offset, sz); });
    }

    _ap_force_inline void set_index_buffer_data(graphics_id_t ibo, u32* indices, count_t offset, count_t count) override {
        _thread_server->queue_task(_tid, [this, ibo, indices, offset, count]() { _inst.set_index_buffer_data(ibo, indices, offset, count); });
    }
    _ap_force_inline void set_uniform_buffer_data(graphics_id_t ubo, const char* name, void* data) override {
        _thread_server->queue_task(_tid, [this, ubo, name, data]() { _inst.set_uniform_buffer_data(ubo, name, data); });
    }

    _ap_force_inline void* map_vertex_buffer_data(graphics_id_t vbo) override {
        void* ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, vbo]() { ret = _inst.map_vertex_buffer_data(vbo); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }
    _ap_force_inline u32* map_index_buffer_data(graphics_id_t ibo) override {
        u32* ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, ibo]() { ret = _inst.map_index_buffer_data(ibo); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }
    _ap_force_inline void* map_uniform_buffer_data(graphics_id_t ubo) override {
        void* ret = 0;
        _thread_server->queue_task(_tid, [this, &ret, ubo]() { ret = _inst.map_uniform_buffer_data(ubo); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline void unmap_vertex_buffer_data(graphics_id_t vbo) override {
        _thread_server->queue_task(_tid, [this, vbo]() { _inst.unmap_vertex_buffer_data(vbo); });
    }
    _ap_force_inline void unmap_index_buffer_data(graphics_id_t ibo) override {
        _thread_server->queue_task(_tid, [this, ibo]() { _inst.unmap_index_buffer_data(ibo); });
    }
    _ap_force_inline void unmap_uniform_buffer_data(graphics_id_t ubo) override {
        _thread_server->queue_task(_tid, [this, ubo]() { _inst.unmap_uniform_buffer_data(ubo); });
    }

    _ap_force_inline Windows_Context* get_windows_context() override {
        Windows_Context* ret = 0;
        _thread_server->queue_task(_tid, [this, &ret]() { ret = _inst.get_windows_context(); });
        _thread_server->wait_for_thread(_tid);
        return ret;
    }

    _ap_force_inline void use_imgui_context() override {
        _thread_server->queue_task(_tid, [this]() { _inst.use_imgui_context(); });
    }

    _ap_force_inline void update_imgui() override {
        _thread_server->queue_task(_tid, [this]() { _inst.update_imgui(); });
    }
    _ap_force_inline void render_imgui() override {
        _thread_server->queue_task(_tid, [this]() { _inst.render_imgui(); });
    }

    inline void _send_debug_message(graphics_enum_t severity, const char* fn, const char* fmt, ...) override {
        if (debug_callback) {
            char buffer[4096];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);
            
            debug_callback(severity, buffer, fn);
        }
    }

    api_t _inst;
    Thread_Server* _thread_server = NULL;
    thread_id_t _tid;
    bool _should_delete_thread_server = false;
};

#define _impl_api \
void set_culling(graphics_enum_t value);\
void set_blending(bool value);\
graphics_id_t make_vertex_array(const Buffer_Layout_Specification& layout);\
graphics_id_t make_shader_source(graphics_enum_t source_type, const char* src);\
graphics_id_t make_shader(graphics_id_t vert_shader, graphics_id_t frag_shader, const Buffer_Layout_Specification& input_layout);\
graphics_id_t make_vertex_buffer(void* data, size_t sz, graphics_enum_t usage);\
graphics_id_t make_index_buffer(u32* indices, count_t count, graphics_enum_t usage);\
graphics_id_t make_uniform_buffer(void* data, const Buffer_Layout_Specification& layout, graphics_enum_t usage);\
graphics_id_t make_texture(graphics_enum_t usage);\
graphics_id_t make_render_target(mz::ivec2 size);\
void destroy_vertex_array(graphics_id_t vao);\
void destroy_shader(graphics_id_t shader);\
void destroy_shader_source(graphics_id_t shader_source);\
void destroy_vertex_buffer(graphics_id_t vbo);\
void destroy_index_buffer(graphics_id_t ibo);\
void destroy_uniform_buffer(graphics_id_t ubo);\
void destroy_texture(graphics_id_t texture);\
void destroy_render_target(graphics_id_t render_target);\
void set_texture_data(graphics_id_t texture, byte* data, mz::ivec2 size, graphics_enum_t fmt);\
void set_texture_wrapping(graphics_id_t texture, graphics_enum_t wrap_mode);\
void set_texture_filtering(graphics_id_t texture, graphics_enum_t min_filter_mode, graphics_enum_t mag_filter_mode);\
void set_texture_multisampling(graphics_id_t texture, u32 count, u32 quality);\
void bind_texture_to_slot(graphics_id_t texture, graphics_enum_t slot);\
void* get_native_texture_handle(graphics_id_t texture);\
void set_render_target_size(graphics_id_t render_target, mz::ivec2 size);\
mz::ivec2 get_render_target_size(graphics_id_t render_target);\
graphics_id_t get_render_target_texture(graphics_id_t render_target);\
void set_clear_color(mz::color16 color, graphics_id_t render_target);\
void set_viewport(mz::viewport viewport);\
void clear(graphics_enum_t clear_flags, graphics_id_t render_target);\
void draw_indices(graphics_id_t vao, graphics_id_t shader, u32 index_count, graphics_id_t ubo, graphics_enum_t draw_mode, graphics_id_t render_target);\
void associate_vertex_buffer(graphics_id_t vbo, graphics_id_t vao);\
void associate_index_buffer(graphics_id_t ibo, graphics_id_t vao);\
void set_vertex_buffer_data(graphics_id_t vbo, void* data, size_t offset, size_t sz);\
void set_index_buffer_data(graphics_id_t ibo, u32* indices, count_t offset, count_t count);\
void set_uniform_buffer_data(graphics_id_t ubo, const char* name, void* data);\
void* map_vertex_buffer_data(graphics_id_t vbo)  ;\
u32* map_index_buffer_data(graphics_id_t ibo)   ;\
void* map_uniform_buffer_data(graphics_id_t ubo) ;\
void unmap_vertex_buffer_data(graphics_id_t vbo) ;\
void unmap_index_buffer_data(graphics_id_t ibo)  ;\
void unmap_uniform_buffer_data(graphics_id_t ubo);\
void update_imgui();\
void render_imgui();\
void use_imgui_context();

namespace ImGui {
    inline void UseGraphicsContext(Graphics_Context* ctx) {
        ctx->use_imgui_context();
    }
}