#include "apparatus.h"
#include "2d_sprite_renderer.h"

#include "texture_asset.h"
#include "sprite_animation_asset_2d.h"

#include "2d_physics/2d_physics.h"

#include "asset_manager/asset_manager.h"

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


void submit(Graphics_Context* graphics, fmat4 transform, fvec2 origin, color16 tint, asset_id_t texture_asset, u32& used_textures, u32& vertex_count, u32& index_count, fvec2 uvs[4], graphics_id_t render_target, Render_Context<Vertex, BUDGET>& context) {
	transform.translate(-origin);
	static fvec2 offset_translation = fvec2(0, 0);

	int texture_index = -1;
	mz::fvec2 size;

	bool texture_is_valid = false;
	
	if (g_asset_manager) {
		texture_is_valid = g_asset_manager->validate(&texture_asset);
	}

	if (texture_is_valid) {
		Asset* asset = g_asset_manager->view(texture_asset);
		bool is_responsible_for_asset = !asset->in_use;

		if (is_responsible_for_asset) g_asset_manager->begin_use(texture_asset);
		ap_assert(asset != NULL, "Asset should not be able to be invalid here");
		ap_assert(asset->is("Texture"), "Asset should not be able to be anything else than texture here");
		ap_assert(asset->in_memory, "Asset should always be in memory after requesting to use it");

		if (used_textures == 32) {
			get_thread_server().wait_for_thread(get_graphics_thread());
			graphics->set_vertex_buffer_data(context.vbo, context.data, 0, vertex_count * sizeof(Vertex));

			graphics->draw_indices(context.vao, context.shader, index_count, context.ubo, G_DRAW_MODE_TRIANGLES, render_target);

			context.data_ptr = context.data;
			index_count = 0;
			vertex_count = 0;
		}

		Texture_Data* tex = asset->get_runtime_data<Texture_Data>();
		texture_index = used_textures++;
		size = tex->size;

		graphics->bind_texture_to_slot(tex->graphics_id, (graphics_enum_t)texture_index);

		if (is_responsible_for_asset) g_asset_manager->end_use(texture_asset);
	}


	// bottom left
	context.data_ptr->pos = (fvec3)(ivec3)transform.get_translation();
	context.data_ptr->color = tint;
	context.data_ptr->uv = uvs[0];
	context.data_ptr->texture_index = (f32)texture_index;
	context.data_ptr++;

	offset_translation.x = size.x;
	offset_translation.y = 0;
	transform.translate(offset_translation);

	// bottom right
	context.data_ptr->pos = (fvec3)(ivec3)transform.get_translation();
	context.data_ptr->color = tint;
	context.data_ptr->uv = uvs[1];
	context.data_ptr->texture_index = (f32)texture_index;
	context.data_ptr++;

	offset_translation.x = 0;
	offset_translation.y = size.y;
	transform.translate(offset_translation);

	// top right
	context.data_ptr->pos = (fvec3)(ivec3)transform.get_translation();
	context.data_ptr->color = tint;
	context.data_ptr->uv = uvs[2];
	context.data_ptr->texture_index = (f32)texture_index;
	context.data_ptr++;

	offset_translation.x = -size.x;
	offset_translation.y = 0;
	transform.translate(offset_translation);

	// top left
	context.data_ptr->pos = (fvec3)(ivec3)transform.get_translation();
	context.data_ptr->color = tint;
	context.data_ptr->uv = uvs[3];
	context.data_ptr->texture_index = (f32)texture_index;
	context.data_ptr++;

	offset_translation.x = 0;
	offset_translation.y = 0;

	index_count += 6;
	vertex_count += 4;
}

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
	
	{
		// Submit normal sprites

		reg.sort<Sprite2D>([](const Sprite2D& lhs, const Sprite2D& rhs) {
			return lhs.depth_level < rhs.depth_level;
		});

		auto view = reg.view<Transform2D, Sprite2D, Entity_Info>();
		for (auto it = view.rbegin(); it != view.rend(); ++it) {
			Transform2D& transform = reg.get<Transform2D>(*it);
			Sprite2D& sprite = reg.get<Sprite2D>(*it);

			static fvec2 uvs[4] = {
				{ 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 }
			};
			submit(graphics, transform.to_mat4(), sprite.origin, sprite.tint, sprite.texture, used_textures, vertex_count, index_count, uvs, render_target, context);
		}
	}

	{
		// Submit sprite animations

		reg.sort<SpriteAnimation2D>([](const SpriteAnimation2D& lhs, const SpriteAnimation2D& rhs) {
			return lhs.depth_level < rhs.depth_level;
		});

		auto view = reg.view<Transform2D, SpriteAnimation2D, Entity_Info>();
		for (auto it = view.rbegin(); it != view.rend(); ++it) {
			Transform2D& transform = reg.get<Transform2D>(*it);
			SpriteAnimation2D& sprite_anim = reg.get<SpriteAnimation2D>(*it);

			if (g_asset_module) {
				auto functions = (Asset_Manager_Function_Library*)g_asset_module->get_function_library();

				
				Texture_Sheet_Runtime_Data sheet_data;
				if (!sprite_anim.get_sheet_data(&sheet_data)) {
					continue;
				}

				if (!functions->validate(&sheet_data.texture)) {
					continue;
				}

				Asset* texture_asset = functions->begin_use(sheet_data.texture);
				if (!texture_asset) {
					continue;
				}

				auto& texture_data = *texture_asset->get_runtime_data<Texture_Data>();
				
				uvec2 ncells = (texture_data.size / sheet_data.cell_size);

				if (ncells.x > 0 && ncells.y > 0) {
					if (sprite_anim.current_frame > sprite_anim.target_frames.max - sprite_anim.target_frames.min) 
						sprite_anim.current_frame = 0;
					u32 xindex = (sprite_anim.target_frames.min + sprite_anim.current_frame) % ncells.width;
					u32 yindex = (sprite_anim.target_frames.min + sprite_anim.current_frame) / ncells.width;
					fvec2 offset(xindex * sheet_data.cell_size.width, yindex * sheet_data.cell_size.height);


					f32 left = offset.x / texture_data.size.width;
					f32 right = (offset.x + sheet_data.cell_size.width) / texture_data.size.width;
					f32 bottom = offset.y / texture_data.size.height;
					f32 top = (offset.y + sheet_data.cell_size.height) / texture_data.size.height;
					fvec2 uvs[4] = {
						{left, top }, { right, top }, { right, bottom }, { left, bottom }
					};
					if (sprite_anim.xflip) {
						f32 temp = uvs[0].x;
						uvs[0].x = uvs[1].x;
						uvs[1].x = temp;
						temp = uvs[2].x;
						uvs[2].x = uvs[3].x;
						uvs[3].x = temp;
					}
					fmat4 transform_matrix = mz::transformation::translation<f32>(transform.position);
					transform_matrix.rotate(transform.rotation, { 0, 0, -1 });
					transform_matrix.scale(transform.scale - fvec2(1));
					transform_matrix.translate(-sprite_anim.origin);
					transform_matrix.scale(fvec2(right - left, top - bottom) - fvec2(1));
					submit(graphics, transform_matrix, fvec2(0), sprite_anim.tint, sheet_data.texture, used_textures, vertex_count, index_count, uvs, render_target, context);
				}

				functions->end_use(sheet_data.texture);
			}
		}
	}

	if (vertex_count > 0) {
		ap_assert(vertex_count <= MAX_VERTICES, "Renderer memory overflow");
		get_thread_server().wait_for_thread(get_graphics_thread());
		graphics->set_vertex_buffer_data(context.vbo, context.data, 0, vertex_count * sizeof(Vertex));

		graphics->draw_indices(context.vao, context.shader, index_count, context.ubo, G_DRAW_MODE_TRIANGLES, render_target);
	}
}

module_scope {
	module_function(void) on_load() {
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

		register_entity_preset("Sprite 2D", [](entt::registry& reg, entt::entity entity) {
			reg.emplace<Transform2D>(entity);
			reg.emplace<Sprite2D>(entity);
		});

		register_entity_preset("Sprite Animation 2D", [](entt::registry& reg, entt::entity entity) {
			reg.emplace<Transform2D>(entity);
			reg.emplace<SpriteAnimation2D>(entity);
		});

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
		spec.creatable = false;
		spec.runtime_data_size =  asset::texture::runtime_data_size;
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
		spec.runtime_data_size =  asset::texture_sheet_2d::runtime_data_size;
		g_asset_manager->register_loader(spec);

		spec = Asset_Loader_Specification();
		spec.extensions =         asset::sprite_animation_2d_preset::extensions;
		spec.tell_size =          asset::sprite_animation_2d_preset::tell_size;
		spec.load =               asset::sprite_animation_2d_preset::load;
		spec.unload =             asset::sprite_animation_2d_preset::unload;
		spec.on_gui =             asset::sprite_animation_2d_preset::on_gui;
		spec.set_default_params = asset::sprite_animation_2d_preset::set_default_params;
		spec.icon =               asset::sprite_animation_2d_preset::icon;
		spec.param_size =         asset::sprite_animation_2d_preset::param_size;
		strcpy(spec.name,         asset::sprite_animation_2d_preset::name);
		spec.creatable = true;
		spec.runtime_data_size =  asset::sprite_animation_2d_preset::runtime_data_size;
		g_asset_manager->register_loader(spec);
	}

	module_function(void) on_unload() {

		unregister_entity_preset("Sprite 2D");
		unregister_entity_preset("Sprite Animation 2D");

		g_asset_manager->unregister_loader(asset::texture::name);
		g_asset_manager->unregister_loader(asset::texture_sheet_2d::name);
		g_asset_manager->unregister_loader(asset::sprite_animation_2d_preset::name);
	}

	module_function(void) on_play_begin() {
		
	}

	module_function(void) on_play_stop() {
		
	}

	module_function(void) save_to_disk(str_ptr_t dir) {
		(void)dir;
        
    }

    module_function(void) load_from_disk(str_ptr_t dir) {
		(void)dir;
		
	}

	module_function(void) on_update(float delta) {
		auto& reg = get_entity_registry();

		auto view = reg.view<SpriteAnimation2D>();
		view.each([delta](SpriteAnimation2D& anim) {
			if (anim.frames_per_second <= 0) {
				anim.time = 0;
			}

			if (anim.is_playing) anim.time += delta;

			if (anim.time >= 1.f / anim.frames_per_second) {
				anim.time -= 1.f / anim.frames_per_second;
				anim.current_frame++;
			}

			if (anim.current_frame > anim.target_frames.max - anim.target_frames.min) anim.current_frame = 0;
		});

		
	}

	module_function(void) on_render() {
		Graphics_Context* graphics = get_graphics();

		if (!g_asset_manager || (uintptr_t)g_asset_manager != (uintptr_t)g_asset_module->get_function_library()) {
			g_asset_manager = (Asset_Manager_Function_Library*)g_asset_module->get_function_library();
		}

		auto& reg = get_entity_registry();

		/*auto windows = get_graphics()->get_windows_context();
		auto wnd = windows->main_window_handle;
		f32 delta = (f32)windows->window_info[wnd].delta_time;*/
		

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

	module_function(void) on_gui() {
		
	}
}