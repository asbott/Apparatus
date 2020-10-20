#include "apparatus.h"
#include "2d_sprite_renderer.h"

#include "texture_asset.h"
#include "sprite_animation_asset_2d.h"

#include "modules/2d_physics/2d_physics.h"

#include "modules/asset_manager/asset_manager.h"

#include "2d_viewport/2d_viewport.h"
#include "2d_editor/2d_editor.h"


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
constexpr size_t TRI_SIZE = sizeof(Vertex) * 3;
constexpr size_t MAX_VERTICES = MAX_BUDGET / sizeof(Vertex);
constexpr size_t MAX_QUADS = MAX_BUDGET / QUAD_SIZE;
constexpr size_t MAX_TRIS = MAX_BUDGET / QUAD_SIZE;
constexpr size_t INDEX_COUNT = MAX_QUADS * 6;

constexpr size_t BUDGET = MAX_VERTICES * sizeof(Vertex);

Module* g_asset_module;
Asset_Manager_Function_Library* g_asset_manager;
Module* g_editor_module;

Render_Context<Vertex, BUDGET> g_quad_render_context;
Render_Context<Vertex, BUDGET> g_editor_render_context;




void render_sprites(Graphics_Context* graphics, const fmat4& view, const fmat4& ortho, const color16& clear_color, graphics_id_t render_target, Render_Context<Vertex, BUDGET>& context) {
	auto& reg = get_entity_registry();

	context.data_ptr = context.data;
	u32 index_count = 0;
	u32 vertex_count = 0;
	
	auto view_inverted = view;
	//view_inverted.translate(-view_size / 2.f);
	view_inverted.rows[2].w = -.5f;
	view_inverted.invert();
	fmat4 cam_transform = ortho * view_inverted;
	graphics->set_uniform_buffer_data(context.ubo, "cam_transform", cam_transform.ptr);

	u32 used_textures = 0;
	
	auto grp = reg.group<Transform2D, Sprite2D>();

	if (grp.size() == 0) return;
	
	grp.sort<Sprite2D>([](const Sprite2D& lhs, const Sprite2D& rhs) {
		return lhs.depth_level < rhs.depth_level;
	}, entt::std_sort());

	for (auto it = grp.end() - 1; it >= grp.begin(); it--) {
		Transform2D& transform = reg.get<Transform2D>(*it);
		Sprite2D& sprite = reg.get<Sprite2D>(*it);

		fmat4 matrix = transform.to_mat4();
		fvec2 origin = sprite.origin;
		matrix.translate(-origin);
		static fvec2 offset_translation = fvec2(0, 0);

		int texture_index = -1;
		mz::fvec2 size;

		bool texture_is_valid = false;
		
		if (g_asset_manager) {
			texture_is_valid = g_asset_manager->validate(&sprite.texture);
		}

		if (texture_is_valid) {
			Asset* asset = g_asset_manager->begin_use(sprite.texture);
			ap_assert(asset != NULL, "Asset should not be able to be invalid here");
			ap_assert(asset->is("Texture"), "Asset should not be able to be anything else than texture here");
			ap_assert(asset->in_memory, "Asset should always be in memory after requesting to use it");

			if (used_textures == 32) {
				get_thread_server().wait_for_thread(get_graphics_thread());
				graphics->set_vertex_buffer_data(context.vbo, context.data, 0, vertex_count * sizeof(Vertex));

				graphics->clear(G_COLOR_BUFFER_BIT, render_target);
				graphics->draw_indices(context.vao, context.shader, index_count, context.ubo, G_DRAW_MODE_TRIANGLES, render_target);

				context.data_ptr = context.data;
				index_count = 0;
				vertex_count = 0;
			}

			Texture_Data* tex = (Texture_Data*)asset->get_runtime_data();
			texture_index = used_textures++;
			size = tex->size;

			graphics->bind_texture_to_slot(tex->graphics_id, (graphics_enum_t)texture_index);

			g_asset_manager->end_use(sprite.texture);
		}


		// bottom left
		context.data_ptr->pos = matrix.get_translation();
		context.data_ptr->color = sprite.tint;
		context.data_ptr->uv = { 0, 1 };
		context.data_ptr->texture_index = (f32)texture_index;
		context.data_ptr++;

		offset_translation.x = size.x;
		offset_translation.y = 0;
		matrix.translate(offset_translation);

		// bottom right
		context.data_ptr->pos = matrix.get_translation();
		context.data_ptr->color = sprite.tint;
		context.data_ptr->uv = { 1, 1 };
		context.data_ptr->texture_index = (f32)texture_index;
		context.data_ptr++;

		offset_translation.x = 0;
		offset_translation.y = size.y;
		matrix.translate(offset_translation);

		// top right
		context.data_ptr->pos = matrix.get_translation();
		context.data_ptr->color = sprite.tint;
		context.data_ptr->uv = { 1, 0 };
		context.data_ptr->texture_index = (f32)texture_index;
		context.data_ptr++;

		offset_translation.x = -size.x;
		offset_translation.y = 0;
		matrix.translate(offset_translation);

		// top left
		context.data_ptr->pos = matrix.get_translation();
		context.data_ptr->color = sprite.tint;
		context.data_ptr->uv = { 0, 0 };
		context.data_ptr->texture_index = (f32)texture_index;
		context.data_ptr++;

		offset_translation.x = 0;
		offset_translation.y = 0;

		index_count += 6;
		vertex_count += 4;
	}

	if (vertex_count > 0) {
		ap_assert(vertex_count <= MAX_VERTICES, "Renderer memory overflow");
		get_thread_server().wait_for_thread(get_graphics_thread());
		graphics->set_vertex_buffer_data(context.vbo, context.data, 0, vertex_count * sizeof(Vertex));

		graphics->draw_indices(context.vao, context.shader, index_count, context.ubo, G_DRAW_MODE_TRIANGLES, render_target);
	}
}

extern "C" {
	_export void __cdecl on_load() {
		Graphics_Context* graphics = get_graphics();

		g_asset_module = get_module("asset_manager");
		g_asset_manager = (Asset_Manager_Function_Library*)g_asset_module->get_function_library();

		g_editor_module = get_module("2d_editor");

		Buffer_Layout_Specification ubo_layout = {
			{ "cam_transform", G_DATA_TYPE_F32MAT4 }
		};

		Buffer_Layout_Specification layout({
			{ "Position", G_DATA_TYPE_FVEC3 },
			{ "UV", G_DATA_TYPE_FVEC2 },
			{ "color", G_DATA_TYPE_FVEC4 },
			{ "texture_index", G_DATA_TYPE_F32 }
		});

		g_quad_render_context.set_shader(graphics, vert_shader_source, frag_shader_source, layout);
		g_editor_render_context.set_shader(graphics, vert_shader_source, frag_shader_source, layout);

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

		g_quad_render_context.set_buffers(graphics, layout, indices, ubo_layout);
		g_editor_render_context.set_buffers(graphics, layout, indices, ubo_layout);

		Asset_Loader_Specification spec;
		spec.extensions =         asset::texture::extensions;
		spec.tell_size =          asset::texture::tell_size;
		spec.load =               asset::texture::load;
		spec.unload =             asset::texture::unload;
		spec.on_gui =             asset::texture::on_gui;
		spec.set_default_params = asset::texture::set_default_params;
		spec.icon =               asset::texture::icon;
		spec.param_size =         asset::texture::param_size;
		strcpy(spec.name,         asset::texture::name);
		g_asset_manager->register_loader(spec);

		spec = Asset_Loader_Specification();
		spec.extensions =         asset::texture_sheet_2d::extensions;
		spec.tell_size =          asset::texture_sheet_2d::tell_size;
		spec.load =               asset::texture_sheet_2d::load;
		spec.unload =             asset::texture_sheet_2d::unload;
		spec.on_gui =             asset::texture_sheet_2d::on_gui;
		spec.set_default_params = asset::texture_sheet_2d::set_default_params;
		spec.icon =               asset::texture_sheet_2d::icon;
		spec.param_size =         asset::texture_sheet_2d::param_size;
		strcpy(spec.name,         asset::texture_sheet_2d::name);
		spec.creatable = true;
		g_asset_manager->register_loader(spec);

		
	}

	_export void __cdecl on_unload() {
		
	}

	_export void __cdecl on_play_begin() {
		
	}

	_export void __cdecl on_play_stop() {
		
	}

	_export void __cdecl save_to_disk(str_ptr_t dir) {
		(void)dir;
        
    }

    _export void __cdecl load_from_disk(str_ptr_t dir) {
		(void)dir;
		
	}

	_export void __cdecl on_update(float delta) {
		(void)delta;

		
	}

	_export void __cdecl on_render() {
		Graphics_Context* graphics = get_graphics();

		if (!g_asset_manager) {
			g_asset_manager = (Asset_Manager_Function_Library*)g_asset_module->get_function_library();
		}

		auto& reg = get_entity_registry();

		reg.view<View2D, Transform2D>().each([graphics, &reg](entt::entity entity, View2D& view, Transform2D& view_transform) {

			auto render_target_size = graphics->get_render_target_size(view.render_target);
			graphics->set_viewport(mz::viewport(0, 0, render_target_size.width, render_target_size.height));
			render_sprites(graphics, view_transform.to_mat4(), view.ortho, view.clear_color, view.render_target, g_quad_render_context);
		});

		if (g_editor_module) {
			auto editor2d = (Editor_2D_Function_Library*)g_editor_module->get_function_library();
			if (editor2d) {
				auto& editor_view = *editor2d->get_view();

				auto render_target_size = graphics->get_render_target_size(editor_view.render_target);
				graphics->set_viewport(mz::viewport(0, 0, render_target_size.width, render_target_size.height));
				render_sprites(graphics, editor_view.transform, editor_view.ortho, editor_view.clear_color, editor_view.render_target, g_editor_render_context);
			}
		}
	}

	_export void __cdecl on_gui() {
		
	}

	_export void* __cdecl _request(void* preq) {
		return NULL;
	}
}