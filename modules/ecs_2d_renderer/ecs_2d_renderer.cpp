#include "apparatus.h"
#include "ecs_2d_renderer.h"

#include "modules/2d_physics/2d_physics.h"

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

constexpr char gizmo_vert_source[] = R"(
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

constexpr char gizmo_frag_source[] = R"(
#version 450 core

out vec4 output_color;

in vec4 vs_color;

void main()
{
    output_color = vs_color;
}

)";

struct Vertex {
	fvec3 pos;
	fvec2 uv;
	color16 color;
	float texture_index;
};

struct Gizmo_Vertex {
	fvec3 pos;
	color16 color;
};

constexpr size_t MAX_BUDGET = 1024 * 1000 * 20;
constexpr size_t QUAD_SIZE = sizeof(Vertex) * 4;
constexpr size_t MAX_VERTICES = MAX_BUDGET / sizeof(Vertex);
constexpr size_t MAX_QUADS = MAX_BUDGET / QUAD_SIZE;
constexpr size_t INDEX_COUNT = MAX_QUADS * 6;

constexpr size_t BUDGET = MAX_VERTICES * sizeof(Vertex);

Graphics_Context* g_graphics;

template <typename vertex_t>
struct Render_Context {

	Render_Context() {
		data = (vertex_t*)malloc(BUDGET);
		data_ptr = data;
	}

	~Render_Context() {
		free(data);

		g_graphics->destroy_shader(shader);
		g_graphics->destroy_vertex_buffer(vbo);
		g_graphics->destroy_index_buffer(ibo);
		g_graphics->destroy_uniform_buffer(ubo);
		g_graphics->destroy_vertex_array(vao);
	}

	void set_shader(Graphics_Context* graphics, str_ptr_t vert_src, str_ptr_t frag_src, Buffer_Layout_Specification& layout) {
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



Module* g_asset_module;


Render_Context<Vertex> quad_render_context;
Render_Context<Vertex> editor_render_context;
Render_Context<Gizmo_Vertex> gizmo_render_context;


struct Gizmo {
	mz::color color = mz::COLOR_WHITE;
};

struct Gizmo_Line : public Gizmo {
	fvec2 a, b;
};

struct Gizmo_Primitive : public Gizmo {
	bool fill;
	mz::color fill_color;
};

struct Gizmo_Circle : Gizmo_Primitive {
	fvec2 pos;
	f32 radius;
};

struct Gizmo_Quad : Gizmo_Primitive {
	fquad quad;
};

struct Gizmo_List {

	inline void add_line(fvec2 a, fvec2 b, color color = COLOR_WHITE) {
		if (!enable) return;
		Gizmo_Line line;
		line.a = a;
		line.b = b;
		line.color = color;
		lines.push_back(line);
	}
	inline void add_wire_circle(fvec2 pos, f32 radius, const color& color = COLOR_WHITE) {
		if (!enable) return;
		Gizmo_Circle circle;
		circle.color = color;
		circle.fill = false;
		circle.pos = pos;
		circle.radius = radius;
		circles.push_back(circle);
	}
	inline void add_filled_circle(fvec2 pos, f32 radius, const color& color = COLOR_WHITE, mz::color fill_color = mz::color(.3f, .3f, .3f, .5f)) {
		if (!enable) return;
		Gizmo_Circle circle;
		circle.color = color;
		circle.fill = true;
		circle.fill_color = fill_color;
		circle.pos = pos;
		circle.radius = radius;
		circles.push_back(circle);
	}
	inline void add_wire_quad(const fquad& quad, const color& color = COLOR_WHITE) {
		if (!enable) return;
		Gizmo_Quad gizmo_quad;
		gizmo_quad.color = color;
		gizmo_quad.fill = false;
		gizmo_quad.quad = quad;
		quads.push_back(gizmo_quad);
	}
	inline void add_filled_quad(const fquad& quad, const color& color = COLOR_WHITE, mz::color fill_color = mz::color(.3f, .3f, .3f, .5f)) {
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

	Dynamic_Array<Gizmo_Line> lines;
	Dynamic_Array<Gizmo_Circle> circles;
	Dynamic_Array<Gizmo_Quad> quads;

	bool enable = true;
};

constexpr mz::fvec2 game_size = mz::fvec2(1600, 900);
fmat4 g_game_ortho;

Gui_Window g_viewport    = { true, "2D Viewport" };
Gui_Window g_editor_view = { true, "2D Editor" };

entt::entity selected_camera = entt::null;
name_str_t selected_camera_name = "";

Gizmo_List gizmo_list;

Hash_Set<graphics_id_t> render_targets_to_destroy;

graphics_id_t g_editor_target;
fmat4 g_editor_transform;
fmat4 g_editor_ortho;
color16 g_editor_clear_color = color16(.3f, .1f, .4f, 1.f);

void render_sprites(Graphics_Context* graphics, const fmat4& view, const fmat4& ortho, const color16& clear_color, graphics_id_t render_target, Render_Context<Vertex>& context, fvec2 view_size) {
	(void)ortho; (void)view_size;
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

	graphics->set_clear_color(clear_color, render_target);

	u32 used_textures = 0;

	reg.view<Transform2D, Sprite2D>().each([graphics, &index_count, &vertex_count, &used_textures, render_target, &context](Transform2D& transform, Sprite2D& sprite) {
		fmat4 matrix = transform.to_mat4();
		fvec2 origin = sprite.origin;
		matrix.translate(-origin);
		static fvec2 offset_translation = fvec2(0, 0);

		int texture_index = -1;
		mz::fvec2 size;

		Asset_Request_Check_If_Valid valid_request;
		valid_request.asset_id = sprite.texture;
		bool texture_is_valid = g_asset_module 
			&& g_asset_module->is_loaded 
			&& g_asset_module->request<bool>(&valid_request);

		if (texture_is_valid) {
			Asset_Request_Begin_Use_asset use_request;
			use_request.asset_id = sprite.texture;
			Asset* asset = g_asset_module->request<Asset*>(&use_request);
			ap_assert(asset != NULL, "Asset should not be able to be invalid here");
			ap_assert(asset->asset_type == ASSET_TYPE_TEXTURE, "Asset should not be able to be anything else than texture here");
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

			Asset_Request_End_Use_Asset end_use_request;
			end_use_request.asset_id = sprite.texture;
			g_asset_module->request<void*>(&end_use_request);
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
	});

	if (vertex_count > 0) {
		ap_assert(vertex_count <= MAX_VERTICES, "Renderer memory overflow");
		get_thread_server().wait_for_thread(get_graphics_thread());
		graphics->set_vertex_buffer_data(context.vbo, context.data, 0, vertex_count * sizeof(Vertex));

		graphics->draw_indices(context.vao, context.shader, index_count, context.ubo, G_DRAW_MODE_TRIANGLES, render_target);
	}
}

frect calculate_view_rect(fvec2 resolution) {
	auto min = ImGui::GetWindowContentRegionMin();
	auto max = ImGui::GetWindowContentRegionMax();

	auto& style = ImGui::GetStyle();

	mz::fvec2 region_size = (fvec2)max - (fvec2)min;
	region_size.x += style.WindowPadding.x * 2;
	region_size.y += style.WindowPadding.y * 2;

	ImVec2 window_size = ImGui::GetWindowSize();

	mz::fvec2 pos = { 0, window_size.y - region_size.y };

	fvec2 sz = resolution;

	if (resolution.x > resolution.y)
	{
		sz.x = region_size.x;
		float ratio = (f32)resolution.y / (f32)resolution.x;
		sz.y = sz.x * ratio;

		if (region_size.y < sz.y)
		{
			sz.y = region_size.y;
			ratio = (f32)resolution.x / (f32)resolution.y;
			sz.x = sz.y * ratio;
			pos.x += (region_size.x - sz.x) / 2.f;
		}
		else 
		{
			pos.y += (region_size.y - sz.y) / 2.f;
		}
	}
	else 
	{
		sz.y = region_size.y;
		float ratio = (f32)resolution.x / (f32)resolution.y;
		sz.x = sz.y * ratio;

		if (region_size.x < sz.x)
		{
			sz.x = region_size.x;
			ratio = (f32)resolution.y / (f32)resolution.x;
			sz.y = sz.x * ratio;
			pos.y += (region_size.y - sz.y) / 2.f;
		}
		else 
		{
			pos.x += (region_size.x - sz.x) / 2.f;
		}
	}

	return frect(pos, sz);
}

fquad get_selection_quad(entt::registry& reg, entt::entity entity, Transform2D& transform) {
	fquad selection_quad = 0;
	if (reg.has<Sprite2D>(entity)) {
		auto& sprite = reg.get<Sprite2D>(entity);
		Asset_Request_Check_If_Valid valid_request;
		valid_request.asset_id = sprite.texture;

		if (g_asset_module && g_asset_module->request<bool>(&valid_request)) {
			Asset_Request_Begin_Use_asset use_request;
			use_request.asset_id = sprite.texture;

			if (Asset* asset = g_asset_module->request<Asset*>(&use_request)) {
				if (asset->asset_type != ASSET_TYPE_TEXTURE) return 0;
				auto* texture_data = (Texture_Data*)asset->get_runtime_data();
				fvec2 size = texture_data->size;

				fmat4 matrix = transform.to_mat4();
				matrix.translate(-sprite.origin);
				selection_quad = fquad(
					matrix * fvec4(fvec2(0.f,    0.f), 1, 1),
					matrix * fvec4(fvec2(0.f,    size.y), 1, 1),
					matrix * fvec4(fvec2(size.x, size.y), 1, 1),
					matrix * fvec4(fvec2(size.x, 0.f), 1, 1)
				);

				Asset_Request_End_Use_Asset end_use_request;
				end_use_request.asset_id = sprite.texture;
				g_asset_module->request<void>(&end_use_request);
			}
		}
	} 

	if (selection_quad == fquad(0) && reg.has<CollisionShape2D>(entity)) {
		auto& shape = reg.get<CollisionShape2D>(entity);
		fvec2 hs = 0;
		if (shape.shape_type == SHAPE_2D_RECT) {
			hs = shape.half_extents;
		} else if (shape.shape_type == SHAPE_2D_RECT) {
			hs = shape.half_extents.x;
		} else {
			ap_assert(false, "Unhandled shape type");
		}
		fmat4 matrix = transform.to_mat4();
		matrix.translate(shape.offset);
		selection_quad = fquad(
			matrix * fvec4(fvec2(-hs.x, -hs.y), 1, 1),
			matrix * fvec4(fvec2(-hs.x,  hs.y), 1, 1),
			matrix * fvec4(fvec2( hs.x,  hs.y), 1, 1),
			matrix * fvec4(fvec2( hs.x, -hs.y), 1, 1)
		);
	}

	return selection_quad;
}

extern "C" {
	_export void __cdecl on_load(Graphics_Context* graphics) {
		g_graphics = graphics;

		register_gui_window(&g_viewport);
		register_gui_window(&g_editor_view);

		g_game_ortho = mz::projection::ortho<float>(-game_size.width / 2.f, game_size.width / 2.f, -game_size.height / 2.f, game_size.height / 2.f, -1, 1);
		g_editor_ortho = mz::projection::ortho<float>(-1920 / 2.f, 1920 / 2.f, -1080 / 2.f, 1080 / 2.f, -1, 1);

		graphics->set_clear_color(mz::color(.35f, .1f, .65f, 1.f));

		graphics->set_culling(G_CULL_NONE);
		graphics->set_blending(true);

		Buffer_Layout_Specification ubo_layout = {
			{ "cam_transform", G_DATA_TYPE_F32MAT4 }
		};

		// Gizmo rendering context

		{
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


		// Quad rendering context

		{
			Buffer_Layout_Specification layout({
				{ "Position", G_DATA_TYPE_FVEC3 },
				{ "UV", G_DATA_TYPE_FVEC2 },
				{ "color", G_DATA_TYPE_FVEC4 },
				{ "texture_index", G_DATA_TYPE_F32 }
			});

			quad_render_context.set_shader(graphics, vert_shader_source, frag_shader_source, layout);
			editor_render_context.set_shader(graphics, vert_shader_source, frag_shader_source, layout);

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

			quad_render_context.set_buffers(graphics, layout, indices, ubo_layout);
			editor_render_context.set_buffers(graphics, layout, indices, ubo_layout);
		}

		

		g_asset_module = get_module("asset_manager");

		g_editor_target = graphics->make_render_target({ 1920, 1080 });
	}

	_export void __cdecl on_unload(Graphics_Context* graphics) {
		(void)graphics;

		unregister_gui_window(&g_viewport);

		graphics->destroy_render_target(g_editor_target);
	}

	_export void __cdecl on_play_begin() {
		auto& reg = get_entity_registry();
		if (reg.valid(selected_camera)) {
			strcpy(selected_camera_name, reg.get<Entity_Info>(selected_camera).name);
		} else {
			const auto& view = reg.view<View2D, Transform2D>();
			if (view.size() > 0) {
				selected_camera = view.front();
			} else {
				log_warn("No camera was found in the scene");
			}
		}
	}

	_export void __cdecl on_play_stop() {
		auto& reg = get_entity_registry();

		reg.view<View2D, Entity_Info>().each([](entt::entity entity, View2D& view, Entity_Info& info) {
			(void)view;
			if (strcmp(info.name, selected_camera_name) == 0) {
				selected_camera = entity;
				return;
			}
		});

		for (graphics_id_t render_target : render_targets_to_destroy) {
			g_graphics->destroy_render_target(render_target);
		}
		render_targets_to_destroy.clear();
	}

	_export void __cdecl on_update(float delta) {
		(void)delta;

		
	}

	_export void __cdecl on_render(Graphics_Context* graphics) {
		(void)graphics;

		auto& reg = get_entity_registry();

		reg.view<CollisionShape2D, Transform2D>().each([](CollisionShape2D& shape, Transform2D& transform) {
			switch (shape.shape_type) {
			case SHAPE_2D_RECT:
			{
				auto& hsz = shape.half_extents;
				auto mat = transform.to_mat4();
				fquad quad = fquad(
					mat * fvec4(fvec2(-hsz.x, -hsz.y), 1, 1)  + shape.offset, 
					mat * fvec4(fvec2(-hsz.x, hsz.y), 1, 1)   + shape.offset, 
					mat * fvec4(fvec2(hsz.x, hsz.y) , 1, 1)   + shape.offset, 
					mat * fvec4(fvec2(hsz.x, -hsz.y), 1 ,1)   + shape.offset
				);
				gizmo_list.add_wire_quad(quad);
				break;
			}
			case SHAPE_2D_CIRCLE:
			{
				auto pos = transform.position + shape.offset;
				gizmo_list.add_wire_circle(pos, shape.half_extents.x * transform.scale.average());	
				break;
			}
			}
		});

		reg.view<View2D, Transform2D>().each([](View2D& , Transform2D transform) {
			fmat4 matrix = transform.to_mat4();
			fvec2 hs = game_size / 2.f;
			gizmo_list.add_wire_quad(fquad(
				(matrix * fvec4(-hs.x,  hs.y, 1, 1)).v2,
				(matrix * fvec4( hs.x,  hs.y, 1, 1)).v2,
				(matrix * fvec4( hs.x, -hs.y, 1, 1)).v2,
				(matrix * fvec4(-hs.x, -hs.y, 1, 1)).v2
			));
		});

		reg.view<View2D, Transform2D>().each([graphics, &reg](View2D& view, Transform2D& view_transform) {
			(void)view_transform;
			if (view.render_target == G_NULL_ID) {
				view.render_target = graphics->make_render_target({ 1, 1 });
				render_targets_to_destroy.emplace(view.render_target);
				graphics_id_t tex = graphics->get_render_target_texture(view.render_target);
				graphics->set_texture_filtering(tex, G_MIN_FILTER_NEAREST, G_MAG_FILTER_NEAREST);
			}
			graphics->set_clear_color(view.clear_color);
			auto render_target_size = graphics->get_render_target_size(view.render_target);
			graphics->set_viewport(mz::viewport(0, 0, render_target_size.width, render_target_size.height));
			graphics->clear(G_COLOR_BUFFER_BIT, view.render_target);
			render_sprites(graphics, view_transform.to_mat4(), g_game_ortho, view.clear_color, view.render_target, quad_render_context, game_size);
		});

		graphics->set_clear_color(g_editor_clear_color);
		auto render_target_size = graphics->get_render_target_size(g_editor_target);
		graphics->set_viewport(mz::viewport(0, 0, render_target_size.width, render_target_size.height));
		graphics->clear(G_COLOR_BUFFER_BIT, g_editor_target);
		render_sprites(graphics, g_editor_transform, g_editor_ortho, g_editor_clear_color, g_editor_target, editor_render_context, graphics->get_render_target_size(g_editor_target));
	
		if (gizmo_list.enable) {
			gizmo_render_context.data_ptr = gizmo_render_context.data;
			u32 index_count = 0;
			u32 vertex_count = 0;

			fmat4 cam_transform = g_editor_transform;
			//auto view_inverted = view.translate(-graphics->get_render_target_size(g_editor_target) / 2.f);
			cam_transform.rows[2].w = -.5f;
			cam_transform.invert();
			cam_transform = g_editor_ortho * cam_transform;
			graphics->set_uniform_buffer_data(gizmo_render_context.ubo, "cam_transform", cam_transform.ptr);

			for (auto& line : gizmo_list.lines) {
				gizmo_render_context.data_ptr->pos = line.a;
				gizmo_render_context.data_ptr->color = line.color;
				gizmo_render_context.data_ptr++;

				gizmo_render_context.data_ptr->pos = line.b;
				gizmo_render_context.data_ptr->color = line.color;
				gizmo_render_context.data_ptr++;

				index_count += 2;
				vertex_count += 2;
			}

			for (auto& quad : gizmo_list.quads) {
				gizmo_render_context.data_ptr->pos = quad.quad.ptr[0];
				gizmo_render_context.data_ptr->color = quad.color;
				gizmo_render_context.data_ptr++;
				gizmo_render_context.data_ptr->pos = quad.quad.ptr[1];
				gizmo_render_context.data_ptr->color = quad.color;
				gizmo_render_context.data_ptr++;

				gizmo_render_context.data_ptr->pos = quad.quad.ptr[1];
				gizmo_render_context.data_ptr->color = quad.color;
				gizmo_render_context.data_ptr++;
				gizmo_render_context.data_ptr->pos = quad.quad.ptr[2];
				gizmo_render_context.data_ptr->color = quad.color;
				gizmo_render_context.data_ptr++;

				gizmo_render_context.data_ptr->pos = quad.quad.ptr[2];
				gizmo_render_context.data_ptr->color = quad.color;
				gizmo_render_context.data_ptr++;
				gizmo_render_context.data_ptr->pos = quad.quad.ptr[3];
				gizmo_render_context.data_ptr->color = quad.color;
				gizmo_render_context.data_ptr++;

				gizmo_render_context.data_ptr->pos = quad.quad.ptr[3];
				gizmo_render_context.data_ptr->color = quad.color;
				gizmo_render_context.data_ptr++;
				gizmo_render_context.data_ptr->pos = quad.quad.ptr[0];
				gizmo_render_context.data_ptr->color = quad.color;
				gizmo_render_context.data_ptr++;

				index_count += 8;
				vertex_count += 8;
			}

			for (auto& circle : gizmo_list.circles) {
				constexpr s32 r = 100;
				for(s32 ii = 0; ii < r; ii++)
				{
					float theta = 4.0f * 3.1415926f * float(ii) / float(r);

					fvec2 vertex_pos = circle.pos + fvec2(circle.radius * cosf(theta), circle.radius * sinf(theta));

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
				
				graphics->draw_indices(gizmo_render_context.vao, gizmo_render_context.shader, index_count, gizmo_render_context.ubo, G_DRAW_MODE_LINES, g_editor_target);
			}

			gizmo_list.clear();
		}
	}

	void do_viewport_gui(Graphics_Context* graphics) {
		static bool is_game_hovered = false;
		ImGuiWindowFlags flags = ImGuiWindowFlags_None;
		if (is_game_hovered) flags |= ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_MenuBar;
		flags |= ImGuiWindowFlags_NoScrollbar;
		flags |= ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::DoGuiWindow(&g_viewport, [graphics]() {
			auto& reg = get_entity_registry();
			ImGui::BeginMenuBar();

			static mz::ivec2 resolution = { 1280, 720 };
			if (ImGui::BeginMenu("Settings")) {
				ImGui::RDragInt2("Resolution", resolution.ptr, 1.f, 1, 20000);

				
				if (ImGui::RBeginCombo("Camera", !reg.valid(selected_camera) ? "None" : reg.get<Entity_Info>(selected_camera).name)) {
					
					if (ImGui::Selectable("None", !reg.valid(selected_camera))) {
						selected_camera = entt::null;
					}

					reg.view<View2D, Transform2D, Entity_Info>().each([](entt::entity entity, View2D& view, Transform2D& transform, Entity_Info& info) {
						(void)transform;(void)view;
						if (ImGui::Selectable(info.name, selected_camera == entity)) {
							selected_camera = entity;
						}
					});

					ImGui::REndCombo();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();

			if (reg.valid(selected_camera)) {
				graphics_id_t fbo = reg.get<View2D>(selected_camera).render_target;

				frect view_rect = calculate_view_rect(resolution);

				auto current_resolution = graphics->get_render_target_size(fbo);

				if (current_resolution != resolution) {
					graphics->set_render_target_size(fbo, resolution);
				}
				
				graphics_id_t texture = graphics->get_render_target_texture(fbo);
				ap_assert(std::abs(view_rect.width / view_rect.height - (f32)resolution.width / (f32)resolution.height) < 0.1f, "{} != {}", view_rect.width / view_rect.height, (f32)resolution.width / (f32)resolution.height);
				ImGui::SetCursorPos(view_rect.pos);
				ImGui::Image
				(
					(ImTextureID)graphics->get_native_texture_handle(texture),
					view_rect.size,
					ImVec2(0, 1),
					ImVec2(1, 0)
				);

				is_game_hovered = ImGui::IsItemHovered();

				auto gui_wnd_pos = (mz::ivec2)ImGui::GetWindowPos();
				mz::ivec2 wnd_pos = ImGui::GetWindowViewport()->Pos;
				mz::ivec2 raw_mouse_pos = ImGui::GetMousePos() - wnd_pos;
				mz::fvec2 game_pos = (gui_wnd_pos - wnd_pos) + view_rect.pos;
				mz::fvec2 mouse_pos = raw_mouse_pos - game_pos;
				mouse_pos.y = view_rect.size.y - mouse_pos.y;

				fvec2 mouse_ratio = { mouse_pos.x / view_rect.size.x, mouse_pos.y / view_rect.size.y };
				mouse_pos.x = mouse_ratio.x * game_size.x;
				mouse_pos.y = mouse_ratio.y * game_size.y;


				fmat4 cam_transform = reg.get<Transform2D>(selected_camera).to_mat4();
				cam_transform.rows[2].w = -.5f;
				fvec2 scale = { cam_transform.rows[0].x, cam_transform.rows[1].y };
				fvec2 view_size = mz::transformation::scale<f32>(scale) * fvec4(game_size, 1, 1);
				fvec2 cam_bot_left = cam_transform.get_translation() - view_size / 2.f;
				fvec2 mouse_world_pos = cam_bot_left + mouse_ratio  * view_size;

				game_input()->state.mouse_pos = mouse_pos;
				game_input()->mouse_world_pos = mouse_world_pos;

				for (int i = AP_KEY_SPACE; i < AP_KEY_COUNT; i++) {
					game_input()->state.keys_press[i] = is_game_hovered && ImGui::IsKeyPressed(i, false);
					game_input()->state.keys_down[i] = is_game_hovered && ImGui::IsKeyDown(i);
				}

				for (int i = AP_MOUSE_BUTTON_1; i < AP_MOUSE_BUTTON_COUNT; i++) {
					game_input()->state.mouse_press[i] = is_game_hovered && ImGui::IsMouseClicked(i);
					game_input()->state.mouse_down[i] = is_game_hovered && ImGui::IsMouseDown(i);
				}
			} else {
				ImGui::Text("No valid entity with View2D and Transform2D selected");
			}

		}, flags);
	}

	void do_editor_view_gui(Graphics_Context* graphics) {
		static bool is_game_hovered = false;
		ImGuiWindowFlags flags = ImGuiWindowFlags_None;
		if (is_game_hovered) flags |= ImGuiWindowFlags_NoMove;
		flags |= ImGuiWindowFlags_MenuBar;
		flags |= ImGuiWindowFlags_NoScrollbar;
		flags |= ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::DoGuiWindow(&g_editor_view, [graphics]() {

			ImGui::BeginMenuBar();

			ImGui::RCheckbox("Gizmos", &gizmo_list.enable);

			ImGui::EndMenuBar();

			auto min = ImGui::GetWindowContentRegionMin();
			auto max = ImGui::GetWindowContentRegionMax();
			fvec2 resolution = { max.x - min.x, max.y - min.y };

			auto& reg = get_entity_registry();
			
			if (resolution.x > 0 && resolution.y > 0) {
				frect view_rect = calculate_view_rect(resolution);
			
				graphics_id_t texture = graphics->get_render_target_texture(g_editor_target);
				
				ImGui::SetCursorPos(view_rect.pos);
				ImGui::Image
				(
					(ImTextureID)graphics->get_native_texture_handle(texture),
					view_rect.size,
					ImVec2(0, 1),
					ImVec2(1, 0)
				);

				g_editor_ortho = mz::projection::ortho<float>((f32)-resolution.width / 2.f, (f32)resolution.width / 2.f, (f32)-resolution.height / 2.f, (f32)resolution.height / 2.f, -1, 1);

				is_game_hovered = ImGui::IsItemHovered();

				auto gui_wnd_pos = (mz::ivec2)ImGui::GetWindowPos();
				mz::ivec2 wnd_pos = ImGui::GetWindowViewport()->Pos;
				mz::ivec2 raw_mouse_pos = ImGui::GetMousePos() - wnd_pos;
				mz::fvec2 game_pos = (gui_wnd_pos - wnd_pos) + view_rect.pos;
				mz::fvec2 mouse_pos = raw_mouse_pos - game_pos;
				mouse_pos.y = view_rect.size.y - mouse_pos.y;

				fvec2 mouse_ratio = { mouse_pos.x / view_rect.size.x, mouse_pos.y / view_rect.size.y };

				mouse_pos.x = (mouse_ratio.x) * (f32)resolution.x;
				mouse_pos.y = (mouse_ratio.y) * (f32)resolution.y;

				fmat4 cam_transform = g_editor_transform;
				//cam_transform.translate(-graphics->get_render_target_size(g_editor_target) / 2.f);
				cam_transform.rows[2].w = -.5f;
				fvec2 scale = { cam_transform.rows[0].x, cam_transform.rows[1].y };
				fvec2 view_size = mz::transformation::scale<f32>(scale) * fvec4(resolution, 1, 1);

				fvec2 cam_bot_left = cam_transform.get_translation() - view_size / 2.f;
				fvec2 mouse_world_pos = cam_bot_left + mouse_ratio  * view_size;

				static bool is_panning = false;
				static fvec2 last_mouse_pos;
				static fvec2 last_mouse_world_pos;
				static fvec2 mouse_down_world_pos;
				static Hash_Map<entt::entity, fvec2> drag_start_positions;

				fvec2 mouse_world_move = mouse_world_pos - last_mouse_world_pos;
				fvec2 mouse_move = last_mouse_pos - mouse_pos;
				last_mouse_pos = mouse_pos;
				last_mouse_world_pos = mouse_world_pos;

				if (ImGui::IsMouseClicked(2) && is_game_hovered) {
					is_panning = true;
				}
				if (ImGui::IsMouseReleased(2)) {
					is_panning = false;
				}

				if (is_panning) {
					g_editor_transform.translate(mouse_move);
				}

				auto windows = graphics->get_windows_context();
				auto wnd = windows->main_window_handle;
				f32 delta = (f32)windows->window_info[wnd].delta_time;

				f32 scroll = -ImGui::GetIO().MouseWheel * delta * 10;

				if (scroll != 0 && is_game_hovered) {
					g_editor_transform.scale(fvec2(scroll));
				}
				
				if (ImGui::IsMouseClicked(0) && is_game_hovered) {
					mouse_down_world_pos = mouse_world_pos;

					bool any_selected = false;
					
					reg.view<Transform2D>().each([&any_selected, &reg, mouse_world_pos](entt::entity entity, Transform2D& transform) {
						if (any_selected) return;
						
						auto click_area = get_selection_quad(reg, entity, transform);

						if (click_area != fquad(0)) {
							fquad mouse_area = {
								mouse_world_pos + fvec2(-.5f, -.5f),
								mouse_world_pos + fvec2(-.5f,  .5f),
								mouse_world_pos + fvec2( .5f,  .5f),
								mouse_world_pos + fvec2( .5f, -.5f)
							};

							if (mz::quads_intersect(click_area, mouse_area)) {
								select_entity(entity);
								any_selected = true;
								return;
							}
						}
					});
					if (!any_selected) deselect_all_entities();

					drag_start_positions.clear();
					for (auto selected_entity : get_selected_entities()) {
						drag_start_positions[selected_entity] = reg.get<Transform2D>(selected_entity).position;
					}
				}

				reg.view<Transform2D, Entity_Info>().each([&reg, mouse_world_pos, mouse_world_move](entt::entity entity, Transform2D& transform, Entity_Info& info) {
					auto click_area = get_selection_quad(reg, entity, transform);

					if (is_game_hovered && click_area != fquad(0)) {
						fquad mouse_area = {
							mouse_world_pos + fvec2(-.5f, -.5f),
							mouse_world_pos + fvec2(-.5f,  .5f),
							mouse_world_pos + fvec2( .5f,  .5f),
							mouse_world_pos + fvec2( .5f, -.5f)
						};

						if (mz::quads_intersect(click_area, mouse_area)) {
							ImGui::BeginTooltip();
							ImVec4 color = is_entity_selected(entity) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
							ImGui::TextColored(color, "%s { %f, %f }", info.name, transform.position.x, transform.position.y);
							ImGui::EndTooltip();
						}
					}

					if (is_game_hovered && ImGui::IsMouseDragging(0)) {
						for (auto selected_entity : get_selected_entities()) {
							if (!reg.valid(selected_entity)) continue;
							auto& start_pos = drag_start_positions[selected_entity];
							auto& selected_transform = reg.get<Transform2D>(selected_entity);

							selected_transform.position = start_pos + mouse_world_pos - mouse_down_world_pos;
						}
					}
				});
			}

		}, flags);
	}

	_export void __cdecl on_gui(Graphics_Context* graphics, ImGuiContext* imgui_ctx) {

		

		ImGui::SetCurrentContext(imgui_ctx);
		
		do_viewport_gui(graphics);
		do_editor_view_gui(graphics);
	}
}