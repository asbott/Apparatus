#pragma once

#include "graphics/graphics_api.h"



template <typename vertex_t, size_t budget>
struct Render_Context {

	static constexpr size_t BUDGET = budget;

	Graphics_Context* _graphics;

	Render_Context() {
		data = (vertex_t*)malloc(budget);
		data_ptr = data;
	}

	~Render_Context() {
		if (data) free(data);

		if (shader != G_NULL_ID) _graphics->destroy_shader(shader);
		if (vbo != G_NULL_ID)    _graphics->destroy_vertex_buffer(vbo);
		if (ibo != G_NULL_ID)    _graphics->destroy_index_buffer(ibo);
		if (ubo != G_NULL_ID)    _graphics->destroy_uniform_buffer(ubo);
		if (vao != G_NULL_ID)    _graphics->destroy_vertex_array(vao);
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
		alignas(16) mz::fmat4 cam_transform;
	} shader_data;

	graphics_id_t shader = G_NULL_ID;
	graphics_id_t ibo = G_NULL_ID;
	graphics_id_t vao = G_NULL_ID;
	graphics_id_t vbo = G_NULL_ID;
	graphics_id_t ubo = G_NULL_ID;

	vertex_t* data = NULL;
	vertex_t* data_ptr;
};



template <size_t  MAX_BUDGET = 1024 * 1000 * 20>
struct Gizmo_Render_Context {
	struct Gizmo_Vertex {
		mz::fvec3 pos;
		mz::color16 color;
	};

	struct Gizmo {
		mz::color color = mz::COLOR_WHITE;
	};

	struct Gizmo_Line : public Gizmo {
		mz::fvec2 a, b;
	};

	struct Gizmo_Primitive : public Gizmo {
		bool fill;
		mz::color fill_color;
	};

	struct Gizmo_Circle : Gizmo_Primitive {
		mz::fvec2 pos;
		f32 radius;
	};

	struct Gizmo_Quad : Gizmo_Primitive {
		mz::fquad quad;
	};
	static constexpr char gizmo_vert_source[] = R"(
#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;

layout (std140) uniform UniformBuffer {
	mat4 cam_transform;
};

out vec4 vs_color;

void main() {
    vs_color = color;

    gl_Position = transpose(cam_transform) * vec4(pos.x, pos.y, pos.z, 1.0);
}

)";

	static constexpr char gizmo_frag_source[] = R"(
#version 450 core

out vec4 output_color;

in vec4 vs_color;

void main()
{
    output_color = vs_color;
}

)";

	static constexpr size_t QUAD_SIZE = sizeof(Gizmo_Vertex) * 4;
	static constexpr size_t TRI_SIZE = sizeof(Gizmo_Vertex) * 3;
	static constexpr size_t MAX_VERTICES = MAX_BUDGET / sizeof(Gizmo_Vertex);
	static constexpr size_t MAX_QUADS = MAX_BUDGET / QUAD_SIZE;
	static constexpr size_t MAX_TRIS = MAX_BUDGET / QUAD_SIZE;
	static constexpr size_t INDEX_COUNT = MAX_QUADS * 6;

	static constexpr size_t BUDGET = MAX_VERTICES * sizeof(Gizmo_Vertex);

	Gizmo_Render_Context() {
		auto graphics = get_graphics();

		Buffer_Layout_Specification ubo_layout = {
			{ "cam_transform", G_DATA_TYPE_F32MAT4 }
		};

		Buffer_Layout_Specification gizmo_layout({
			{ "Position", G_DATA_TYPE_FVEC3 },
			{ "color", G_DATA_TYPE_FVEC4 }
			});

		gizmo_render_context.set_shader(graphics, gizmo_vert_source, gizmo_frag_source, gizmo_layout);

		Dynamic_Array<u32> indices;
		indices.resize(INDEX_COUNT);

		for (u32 i = 0U; i < indices.size(); i++) {
			indices[i] = i;
		}

		gizmo_render_context.set_buffers(graphics, gizmo_layout, indices, ubo_layout);
	}

	inline void add_line(mz::fvec2 a, mz::fvec2 b, mz::color color = mz::COLOR_WHITE) {
		if (!enable) return;
		Gizmo_Line line;
		line.a = a;
		line.b = b;
		line.color = color;
		lines.push_back(line);
	}
	inline void add_wire_circle(mz::fvec2 pos, f32 radius, const mz::color& color = mz::COLOR_WHITE) {
		if (!enable) return;
		Gizmo_Circle circle;
		circle.color = color;
		circle.fill = false;
		circle.pos = pos;
		circle.radius = radius;
		circles.push_back(circle);
	}
	inline void add_filled_circle(mz::fvec2 pos, f32 radius, const mz::color& color = mz::COLOR_WHITE, mz::color fill_color = mz::color(.3f, .3f, .3f, .5f)) {
		if (!enable) return;
		Gizmo_Circle circle;
		circle.color = color;
		circle.fill = true;
		circle.fill_color = fill_color;
		circle.pos = pos;
		circle.radius = radius;
		circles.push_back(circle);
	}
	inline void add_wire_quad(const mz::fquad& quad, const mz::color& color = mz::COLOR_WHITE) {
		if (!enable) return;
		Gizmo_Quad gizmo_quad;
		gizmo_quad.color = color;
		gizmo_quad.fill = false;
		gizmo_quad.quad = quad;
		quads.push_back(gizmo_quad);
	}
	inline void add_filled_quad(const mz::fquad& quad, const mz::color& color = mz::COLOR_WHITE, mz::color fill_color = mz::color(.3f, .3f, .3f, .5f)) {
		if (!enable) return;
		Gizmo_Quad gizmo_quad;
		gizmo_quad.color = color;
		gizmo_quad.fill = true;
		gizmo_quad.fill_color = fill_color;
		gizmo_quad.quad = quad;
		quads.push_back(gizmo_quad);
	}

	inline void clear() {
		lines.clear();
		circles.clear();
		quads.clear();
	}

	inline void flush(const mz::fmat4& cam_transform, graphics_id_t render_target) {
		if (!enable) return;

		auto graphics = get_graphics();

		gizmo_render_context.data_ptr = gizmo_render_context.data;
		u32 index_count = 0;
		u32 vertex_count = 0;
		
		graphics->set_uniform_buffer_data(gizmo_render_context.ubo, "cam_transform", cam_transform.ptr);

		for (auto& line : lines) {
			gizmo_render_context.data_ptr->pos = line.a;
			gizmo_render_context.data_ptr->color = line.color;
			gizmo_render_context.data_ptr++;

			gizmo_render_context.data_ptr->pos = line.b;
			gizmo_render_context.data_ptr->color = line.color;
			gizmo_render_context.data_ptr++;

			index_count += 2;
			vertex_count += 2;
		}

		for (auto& quad : quads) {
			gizmo_render_context.data_ptr->pos = quad.quad.ptr[0];
			gizmo_render_context.data_ptr->pos.z = 999999;
			gizmo_render_context.data_ptr->color = quad.color;
			gizmo_render_context.data_ptr++;
			gizmo_render_context.data_ptr->pos = quad.quad.ptr[1];
			gizmo_render_context.data_ptr->color = quad.color;
			gizmo_render_context.data_ptr++;

			gizmo_render_context.data_ptr->pos = quad.quad.ptr[1];
			gizmo_render_context.data_ptr->pos.z = 999999;
			gizmo_render_context.data_ptr->color = quad.color;
			gizmo_render_context.data_ptr++;
			gizmo_render_context.data_ptr->pos = quad.quad.ptr[2];
			gizmo_render_context.data_ptr->pos.z = 999999;
			gizmo_render_context.data_ptr->color = quad.color;
			gizmo_render_context.data_ptr++;

			gizmo_render_context.data_ptr->pos = quad.quad.ptr[2];
			gizmo_render_context.data_ptr->pos.z = 999999;
			gizmo_render_context.data_ptr->color = quad.color;
			gizmo_render_context.data_ptr++;
			gizmo_render_context.data_ptr->pos = quad.quad.ptr[3];
			gizmo_render_context.data_ptr->pos.z = 999999;
			gizmo_render_context.data_ptr->color = quad.color;
			gizmo_render_context.data_ptr++;

			gizmo_render_context.data_ptr->pos = quad.quad.ptr[3];
			gizmo_render_context.data_ptr->pos.z = 999999;
			gizmo_render_context.data_ptr->color = quad.color;
			gizmo_render_context.data_ptr++;
			gizmo_render_context.data_ptr->pos = quad.quad.ptr[0];
			gizmo_render_context.data_ptr->color = quad.color;
			gizmo_render_context.data_ptr++;

			index_count += 8;
			vertex_count += 8;
		}

		for (auto& circle : circles) {
			constexpr s32 r = 100;
			for(s32 ii = 0; ii < r; ii++)
			{
				float theta = 4.0f * 3.1415926f * float(ii) / float(r);

				mz::fvec2 vertex_pos = circle.pos + mz::fvec2(circle.radius * cosf(theta), circle.radius * sinf(theta));

				if (ii != 0){
					gizmo_render_context.data_ptr->pos = vertex_pos;
					gizmo_render_context.data_ptr->color = circle.color;
					gizmo_render_context.data_ptr++;
					index_count++;
					vertex_count++;
				}

				if (ii != r - 1) {
					gizmo_render_context.data_ptr->pos = vertex_pos;
					gizmo_render_context.data_ptr->color = circle.color;
					gizmo_render_context.data_ptr++;
					index_count++;
					vertex_count++;
				}
			}
		}

		if (vertex_count > 0) {
			ap_assert(vertex_count <= MAX_VERTICES, "Renderer memory overflow");
			get_thread_server().wait_for_thread(get_graphics_thread());
			graphics->set_vertex_buffer_data(gizmo_render_context.vbo, gizmo_render_context.data, 0, vertex_count * sizeof(Gizmo_Vertex));

			graphics->draw_indices(gizmo_render_context.vao, gizmo_render_context.shader, index_count, gizmo_render_context.ubo, G_DRAW_MODE_LINES, render_target);
		}

		clear();
	}

	Dynamic_Array<Gizmo_Line> lines;
	Dynamic_Array<Gizmo_Circle> circles;
	Dynamic_Array<Gizmo_Quad> quads;

	bool enable = true;

	Render_Context<Gizmo_Vertex, BUDGET> gizmo_render_context;
};