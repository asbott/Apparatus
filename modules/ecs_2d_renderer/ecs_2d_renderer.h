#pragma once

#include "apparatus.h"

#include "asset_manager/asset_manager.h"

tag(component)
struct Transform2D {
	tag(property)
	fvec2 position;
	tag(property)
	f32 rotation;
	tag(property)
	fvec2 scale = fvec2(1);

	fmat4 to_mat4() {
		fmat4 m = mz::transformation::translation<f32>(position);
		m.rotate(rotation, { 0, 0, -1 });
		m.scale(scale - fvec2(1));
		return m;
	}
};

tag(component)
struct Sprite2D {

	tag(property, texture)
	asset_id_t texture = G_NULL_ID;

	tag(property, color)
	color tint = COLOR_WHITE;

	tag(property)
	fvec2 origin = 0;
};

tag(component)
struct View2D {
	View2D() {
		render_target = G_NULL_ID;
	}
	tag(property, color)
	color16 clear_color = color16(.3f, .1f, .4f, 1.f);

	graphics_id_t render_target = G_NULL_ID;

	fmat4 ortho;
};

/*struct Texture_Asset {
	graphics_id_t texture;

	inline byte* load(str_ptr_t file, byte* ptr) {
		Texture_Asset

		return ptr;
	}
};*/


template <typename vertex_t, size_t budget>
struct Render_Context {

	static constexpr size_t BUDGET = budget;

	Graphics_Context* _graphics;

	Render_Context() {
		data = (vertex_t*)malloc(budget);
		data_ptr = data;
	}

	~Render_Context() {
		free(data);

		_graphics->destroy_shader(shader);
		_graphics->destroy_vertex_buffer(vbo);
		_graphics->destroy_index_buffer(ibo);
		_graphics->destroy_uniform_buffer(ubo);
		_graphics->destroy_vertex_array(vao);
	}

	void set_shader(Graphics_Context* graphics, str_ptr_t vert_src, str_ptr_t frag_src, Buffer_Layout_Specification& layout) {
		_graphics = graphics;
		graphics_id_t vert_shader = graphics->make_shader_source(G_SHADER_SOURCE_TYPE_VERTEX,   vert_src);
		graphics_id_t frag_shader = graphics->make_shader_source(G_SHADER_SOURCE_TYPE_FRAGMENT, frag_src);
		
		shader = graphics->make_shader(vert_shader, frag_shader, layout);
		
		graphics->destroy_shader_source(vert_shader);
		graphics->destroy_shader_source(frag_shader);
	}

	void set_buffers(Graphics_Context* graphics, Buffer_Layout_Specification& vao_layout, Dynamic_Array<u32>& indices, Buffer_Layout_Specification& ubo_layout) {
		vao = graphics->make_vertex_array(vao_layout);

		vbo = graphics->make_vertex_buffer(data, BUDGET, G_BUFFER_USAGE_DYNAMIC_WRITE);

		ibo = graphics->make_index_buffer(indices.data(), indices.size(), G_BUFFER_USAGE_STATIC_WRITE);

		graphics->associate_vertex_buffer(vbo, vao);
		graphics->associate_index_buffer (ibo, vao);

		ubo = graphics->make_uniform_buffer(&shader_data, ubo_layout, G_BUFFER_USAGE_DYNAMIC_WRITE);
	}

	struct Uniform_Buffer {
		alignas(16) fmat4 cam_transform;
	} shader_data;

	graphics_id_t shader;
	graphics_id_t ibo;
	graphics_id_t vao;
	graphics_id_t vbo;
	graphics_id_t ubo;

	vertex_t* data;
	vertex_t* data_ptr;
};

enum Ecs_2D_Renderer_Request_Type {
	ECS_2D_RENDERER_REQUEST_GET_EDITOR_VIEW
};

struct Ecs_2D_Renderer_Request {
	Ecs_2D_Renderer_Request(Ecs_2D_Renderer_Request_Type t) : type(t) {}
	Ecs_2D_Renderer_Request_Type type;
};

struct Editor_View {
	fmat4 transform;
	fmat4 ortho;
	graphics_id_t render_target;
	fcolor16 clear_color = color16(.3f, .1f, .4f, 1.f);
};