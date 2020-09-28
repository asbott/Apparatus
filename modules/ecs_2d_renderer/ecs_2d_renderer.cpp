#include "apparatus.h"
#include "ecs_2d_renderer.h"

#include "modules/asset_manager/asset_manager.h"


constexpr char vert_shader_source[] = R"(
#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in float texture_index;

layout (std140) uniform UniformBuffer {
	mat4 cam_transform;
};

out vec2 vs_uv;
out vec4 vs_color;
out flat int vs_texture_index;

void main() {
    vs_uv = uv;
    vs_color = color;
    vs_texture_index = int(texture_index);

    gl_Position = transpose(cam_transform) * vec4(pos.x, pos.y, pos.z, 1.0);
}

)";

constexpr char frag_shader_source[] = R"(
#version 450 core

out vec4 output_color;

in vec2 vs_uv;
in vec4 vs_color;
in flat int vs_texture_index;

uniform sampler2D samplers[32];

void main()
{
    //vec4 texture_color = texture(samplers[vs_texture_index], vs_uv);
    //output_color = texture_color * vs_color;
	if (vs_texture_index == -1)
		output_color = vs_color;
	else {
		vec4 texture_color = texture(samplers[vs_texture_index], vs_uv);
		output_color = vs_color * texture_color;
	}
}

)";

struct Vertex {
	fvec3 pos;
	fvec2 uv;
	color16 color;
	float texture_index;
};

constexpr size_t MAX_BUDGET = 1024 * 1000 * 20;
constexpr size_t QUAD_SIZE = sizeof(Vertex) * 4;
constexpr size_t MAX_VERTICES = MAX_BUDGET / sizeof(Vertex);
constexpr size_t MAX_QUADS = MAX_BUDGET / QUAD_SIZE;
constexpr size_t INDEX_COUNT = MAX_QUADS * 6;

constexpr size_t BUDGET = MAX_VERTICES * sizeof(Vertex);

graphics_id_t g_shader;
graphics_id_t g_ibo;
graphics_id_t g_vao;
graphics_id_t g_vbo;
graphics_id_t g_ubo;

Module* g_asset_module;

struct Uniform_Buffer {
	alignas(16) fmat4 cam_transform;
} shader_data;

Vertex* g_data;
Vertex* g_data_ptr;

fmat4 g_ortho;

extern "C" {
	_export void __cdecl on_load(Graphics_Context* graphics) {
		log_info("on_load called in test_module!");
		(void)graphics;

		g_data = (Vertex*)malloc(BUDGET);
		g_data_ptr = g_data;

		g_ortho = mz::projection::ortho<float>(0, 1280, 0, 720, -2, 2);

		graphics->set_clear_color(mz::color(.35f, .1f, .65f, 1.f));

		graphics->set_culling(G_CULL_NONE);
		graphics->set_blending(true);

		Buffer_Layout_Specification layout({
			{ "Position", G_DATA_TYPE_FVEC3 },
			{ "UV", G_DATA_TYPE_FVEC2 },
			{ "color", G_DATA_TYPE_FVEC4 },
			{ "texture_index", G_DATA_TYPE_F32 }
		});

		graphics_id_t vert_shader = graphics->make_shader_source(G_SHADER_SOURCE_TYPE_VERTEX,   vert_shader_source);
		graphics_id_t frag_shader = graphics->make_shader_source(G_SHADER_SOURCE_TYPE_FRAGMENT, frag_shader_source);
		
		g_shader = graphics->make_shader(vert_shader, frag_shader, layout);
		
		graphics->destroy_shader_source(vert_shader);
		graphics->destroy_shader_source(frag_shader);

		g_vao = graphics->make_vertex_array(layout);

		g_vbo = graphics->make_vertex_buffer(g_data, BUDGET, G_BUFFER_USAGE_DYNAMIC_WRITE);

		Dynamic_Array<u32> indices;
		indices.resize(INDEX_COUNT);

		u32 offset = 0U;
		for (u32 i = 0U; i < indices.size(); i += 6U) {
			indices[i] = offset + 0U;
			indices[i + 1U] = offset + 1U;
			indices[i + 2U] = offset + 2U;
			indices[i + 3U] = offset + 2U;
			indices[i + 4U] = offset + 3U;
			indices[i + 5U] = offset + 0U;

			offset += 4;
		}

		g_ibo = graphics->make_index_buffer(indices.data(), indices.size(), G_BUFFER_USAGE_STATIC_WRITE);

		graphics->associate_vertex_buffer(g_vbo, g_vao);
		graphics->associate_index_buffer (g_ibo, g_vao);

		Buffer_Layout_Specification ubo_layout = {
			{ "cam_transform", G_DATA_TYPE_F32MAT4 }
		};

		g_ubo = graphics->make_uniform_buffer(&shader_data, ubo_layout, G_BUFFER_USAGE_DYNAMIC_WRITE);
	
		auto& reg = get_entity_registry();
		
		auto cam = reg.create();
		reg.emplace<View2D>(cam);
		reg.emplace<Transform2D>(cam);
		reg.emplace<Entity_Info>(cam);
		reg.get<Entity_Info>(cam).id = cam;
		name_str_t name = "";
		sprintf(name, "Camera");
		strcpy(reg.get<Entity_Info>(cam).name, name);

		auto entity = reg.create();
		reg.emplace<Entity_Info>(entity);
		reg.get<Entity_Info>(entity).id = entity;
		sprintf(name, "Egg");
		strcpy(reg.get<Entity_Info>(entity).name, name);

		reg.emplace<Transform2D>(entity);
		reg.get<Transform2D>(entity).matrix.translate(fvec2(rand() % (1280 - 64), rand() % (720 - 64)));
		reg.emplace<Sprite2D>(entity);
	}

	_export void __cdecl on_unload(Graphics_Context* graphics) {
		(void)graphics;
		free(g_data);
	}

	_export void __cdecl on_update(float delta) {
		(void)delta;

		if (!g_asset_module) g_asset_module = get_module("asset_manager");
	}

	_export void __cdecl on_render(Graphics_Context* graphics) {
		(void)graphics;
		g_data_ptr = g_data;
		u32 index_count = 0;
		u32 vertex_count = 0;
		auto& reg = get_entity_registry();
		reg.view<View2D, Transform2D>().each([graphics, &index_count, &reg, &vertex_count](View2D& view, Transform2D& view_transform) {
			(void)view_transform;
			
			auto view_inverted = view_transform.matrix;
			view_inverted.rows[2].w = -.5f;
			view_inverted.invert();
			fmat4 cam_transform = g_ortho * view_inverted;
			graphics->set_uniform_buffer_data(g_ubo, "cam_transform", cam_transform.ptr);

			graphics->set_clear_color(view.clear_color);

			reg.view<Transform2D, Sprite2D>().each([graphics, &index_count, &vertex_count](Transform2D& transform, Sprite2D& sprite) {
				
				fmat4 matrix = transform.matrix;
				static fvec2 offset_translation = fvec2(0, 0);

				int texture_index = -1;
				mz::fvec2 size = { 1, 1 };

				Asset_Request_Check_If_Valid valid_request;
				valid_request.asset_id = sprite.texture;
				bool texture_is_valid = g_asset_module->request<bool>(&valid_request);

				if (texture_is_valid) {
					Asset_Request_Begin_Use_asset use_request;
					use_request.asset_id = sprite.texture;
					Asset* asset = g_asset_module->request<Asset*>(&use_request);
					ap_assert(asset != NULL, "Asset should not be able to be invalid here");
					ap_assert(asset->asset_type == ASSET_TYPE_TEXTURE, "Asset should not be able to be anything else than texture here");
					ap_assert(asset->in_memory, "Asset should always be in memory after requesting to use it");

					Texture_Data* tex = (Texture_Data*)asset->get_runtime_data();
					texture_index = tex->graphics_id;
					size = tex->size;

					graphics->bind_texture_to_slot((graphics_id_t)texture_index, (graphics_enum_t)texture_index);
				}
				

				// bottom left
				g_data_ptr->pos = matrix.get_translation();
				g_data_ptr->color = sprite.tint;
				g_data_ptr->uv = { 0, 1 };
				g_data_ptr->texture_index = (f32)texture_index;
				g_data_ptr++;

				offset_translation.x = size.x;
				offset_translation.y = 0;
				matrix.translate(offset_translation);

				// bottom right
				g_data_ptr->pos = matrix.get_translation();
				g_data_ptr->color = sprite.tint;
				g_data_ptr->uv = { 1, 1 };
				g_data_ptr->texture_index = (f32)texture_index;
				g_data_ptr++;

				offset_translation.x = 0;
				offset_translation.y = size.y;
				matrix.translate(offset_translation);

				// top right
				g_data_ptr->pos = matrix.get_translation();
				g_data_ptr->color = sprite.tint;
				g_data_ptr->uv = { 1, 0 };
				g_data_ptr->texture_index = (f32)texture_index;
				g_data_ptr++;

				offset_translation.x = -size.x;
				offset_translation.y = 0;
				matrix.translate(offset_translation);

				// top left
				g_data_ptr->pos = matrix.get_translation();
				g_data_ptr->color = sprite.tint;
				g_data_ptr->uv = { 0, 0 };
				g_data_ptr->texture_index = (f32)texture_index;
				g_data_ptr++;

				offset_translation.x = 0;
				offset_translation.y = 0;

				index_count += 6;
				vertex_count += 4;
			});
		});


		if (vertex_count > 0) {
			ap_assert(vertex_count <= MAX_VERTICES, "Renderer memory overflow");
			get_thread_server().wait_for_thread(get_graphics_thread());
			graphics->set_vertex_buffer_data(g_vbo, g_data, 0, vertex_count * sizeof(Vertex));
			graphics->draw_indices(g_vao, g_shader, index_count, g_ubo);
		}
	}

	_export void __cdecl on_gui(Graphics_Context* graphics, ImGuiContext* imgui_ctx) {
		(void)graphics;(void)imgui_ctx;
		ImGui::SetCurrentContext(imgui_ctx);

		
	}
}