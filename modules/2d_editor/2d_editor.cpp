#include "2d_editor.h"

#include "2d_physics/2d_physics.h"
#include "2d_viewport/2d_viewport.h"
#include "asset_manager/asset_manager.h"
#include "2d_sprite_renderer/2d_sprite_renderer.h"



Editor_View g_editor_cam;

Gui_Window g_editor_view = { true, "2D Editor" };

Gizmo_Render_Context gizmo_list;

Module* g_asset_module;
Asset_Manager_Function_Library* g_asset_manager;

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

fquad get_selection_by_sprite(entt::registry& reg, entt::entity entity, Transform2D& transform) {
	if (reg.has<Sprite2D>(entity)) {
		auto& sprite = reg.get<Sprite2D>(entity);

		if (g_asset_module && g_asset_manager->validate(&sprite.texture)) {
			if (Asset* asset = g_asset_manager->begin_use(sprite.texture)) {
				if (!asset->is("Texture")) return fquad(0);
				auto* texture_data = (Texture_Data*)asset->get_runtime_data();
				fvec2 size = texture_data->size;

				fmat4 matrix = transform.to_mat4();
				matrix.translate(-sprite.origin);
				auto selection_quad = fquad(
					matrix * fvec4(fvec2(0.f,    0.f), 1, 1),
					matrix * fvec4(fvec2(0.f,    size.y), 1, 1),
					matrix * fvec4(fvec2(size.x, size.y), 1, 1),
					matrix * fvec4(fvec2(size.x, 0.f), 1, 1)
				);

				g_asset_manager->end_use(sprite.texture);

				return selection_quad;
			}
		}
	}
	return fquad(0);
}

fquad get_selection_quad(entt::registry& reg, entt::entity entity, Transform2D& transform) {

	fquad selection_quad = get_selection_by_sprite(reg, entity, transform);
	
	if (selection_quad == fquad(0) && reg.has<CollisionShape2D>(entity)) {
		auto& shape = reg.get<CollisionShape2D>(entity);
		fvec2 hs = 0;
		if (shape.shape_type == SHAPE_2D_RECT) {
			hs = shape.half_extents;
		} else if (shape.shape_type == SHAPE_2D_CIRCLE) {
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
	_export void __cdecl on_load() {
		register_gui_window(&g_editor_view);

		g_asset_module = get_module("asset_manager");
		g_asset_manager = (Asset_Manager_Function_Library*)g_asset_module->get_function_library();

		g_editor_cam.render_target = get_graphics()->make_render_target({ 1920, 1080 });
	}

	_export void __cdecl on_unload() {
		Graphics_Context* graphics = get_graphics();

		graphics->destroy_render_target(g_editor_cam.render_target);

		unregister_gui_window(&g_editor_view);
	}

	_export void __cdecl on_play_begin() {

	}

	_export void __cdecl on_play_stop() {

	}

	_export void __cdecl on_update(float delta) {
		(void)delta;

		
	}

	_export void __cdecl on_render() {
		auto graphics = get_graphics();

		graphics->set_clear_color(g_editor_cam.clear_color, g_editor_cam.render_target);
		graphics->clear(G_COLOR_BUFFER_BIT, g_editor_cam.render_target);

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

		reg.view<View2D, Transform2D>().each([graphics, &reg](entt::entity entity, View2D& view, Transform2D& view_transform) {
			fmat4 matrix = view_transform.to_mat4();
			fvec2 hs = view.viewport_size / 2.f;
			gizmo_list.add_wire_quad(fquad(
				(matrix * fvec4(-hs.x,  hs.y, 1, 1)).v2,
				(matrix * fvec4( hs.x,  hs.y, 1, 1)).v2,
				(matrix * fvec4( hs.x, -hs.y, 1, 1)).v2,
				(matrix * fvec4(-hs.x, -hs.y, 1, 1)).v2
			));
		});

		for (auto ent : get_selected_entities()) {
			if (reg.has<Transform2D>(ent)) {
				gizmo_list.add_wire_quad(get_selection_quad(reg, ent, reg.get<Transform2D>(ent)), COLOR_RED);
			}
		}

		if (gizmo_list.enable) {
			gizmo_list.flush();
		}
	}

	_export void __cdecl on_gui() {
		Graphics_Context* graphics = get_graphics();
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

			frect view_rect = calculate_view_rect(resolution);
			if (view_rect .width > 0 && view_rect .height > 0) {

				graphics_id_t texture = graphics->get_render_target_texture(g_editor_cam.render_target);

				ImGui::SetCursorPos(view_rect.pos);
				ImGui::Image
				(
					(ImTextureID)graphics->get_native_texture_handle(texture),
					view_rect.size,
					ImVec2(0, 1),
					ImVec2(1, 0)
				);

				g_editor_cam.ortho = mz::projection::ortho<float>((f32)-resolution.width / 2.f, (f32)resolution.width / 2.f, (f32)-resolution.height / 2.f, (f32)resolution.height / 2.f, -1000000, 1000000);

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

				fmat4 cam_transform = g_editor_cam.transform;
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
					g_editor_cam.transform.translate(mouse_move);
				}

				auto windows = graphics->get_windows_context();
				auto wnd = windows->main_window_handle;
				f32 delta = (f32)windows->window_info[wnd].delta_time;

				f32 scroll = (-ImGui::GetIO().MouseWheel * delta * 10);
				if (scale.magnitude() > 1.f) {
					scroll /= scale.magnitude();
				}

				if (scroll != 0 && is_game_hovered) {
					fvec2 diff_to_mouse = mouse_world_pos - g_editor_cam.transform.get_translation();
					g_editor_cam.transform.translate(diff_to_mouse);
					g_editor_cam.transform.scale(fvec2(scroll));
					g_editor_cam.transform.translate(-diff_to_mouse);
				}

				if (ImGui::IsKeyDown(AP_KEY_F) && is_any_entity_selected()) {
					auto& first_selected = *get_selected_entities().begin();
					if (reg.has<Transform2D>(first_selected)) {
						auto& selected_transform = reg.get<Transform2D>(first_selected);
						g_editor_cam.transform.rows[0].w = selected_transform.position.x;
						g_editor_cam.transform.rows[1].w = selected_transform.position.y;
						g_editor_cam.transform.rows[0].x = 2;
						g_editor_cam.transform.rows[1].y = 2;
					}
				}

				if (ImGui::IsMouseClicked(0) && is_game_hovered) {
					mouse_down_world_pos = mouse_world_pos;

					bool any_selected = false;
					reg.view<Transform2D>().each([&any_selected, &reg, mouse_world_pos](entt::entity entity, Transform2D& transform) {
						if (any_selected) return;

						auto click_area = get_selection_quad(reg, entity, transform);
						fquad mouse_area = {
							mouse_world_pos + fvec2(-.5f, -.5f),
							mouse_world_pos + fvec2(-.5f,  .5f),
							mouse_world_pos + fvec2( .5f,  .5f),
							mouse_world_pos + fvec2( .5f, -.5f)
						};
						
						if (click_area != fquad(0)) {
							

							
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
						if (reg.has<Transform2D>(selected_entity))
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
							if (!reg.valid(selected_entity) || !reg.has<Transform2D>(selected_entity)) continue;
							auto& start_pos = drag_start_positions[selected_entity];
							auto& selected_transform = reg.get<Transform2D>(selected_entity);

							selected_transform.position = start_pos + mouse_world_pos - mouse_down_world_pos;
						}
					}
				});
			}

		}, flags);
	}

	_export void* __cdecl get_function_library() {
		static Editor_2D_Function_Library lib;

		lib.get_view = []() {
			return &g_editor_cam;
		};

		return &lib;
	}
}