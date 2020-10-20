#include "2d_viewport.h"

Hash_Set<ivec2> resolutions_16_9 = {
	{ 128, 72 },
	{ 384, 216 },
	{ 512, 288 },
	{ 640, 360 },
	{ 768, 432 },
	{ 1024, 576 },
	{ 1280, 720 },
	{ 1536, 864 },
	{ 1920, 1080 },
	{ 2560, 1440 },
	{ 3072, 1728 },
	{ 3840, 2160 },
	{ 5120, 2880 },
	{ 6400, 3600 },
	{ 7680, 4320 },
};

Hash_Set<ivec2> resolutions_16_10 = {
	{ 1280, 800 },
	{ 1440, 900 },
	{ 1680, 1050 },
	{ 1920, 1200 },
	{ 2560, 1600 },
	{ 3840, 2400 },
};

Hash_Set<ivec2> resolutions_4_3 = {
	{ 160, 120 },
	{ 320, 240 },
	{ 640, 480 },
	{ 960, 720 },
	{ 1024, 768 },
	{ 1280, 960 },
	{ 1600, 1200 },
	{ 1920, 1440 },
	{ 2560, 1920 },
	{ 3200, 2400 },
	{ 4096, 3072 },
	{ 6400, 4800 },

};

Gui_Window g_viewport    = { true, "2D Viewport" };

Ordered_Set<graphics_id_t> g_render_targets_to_destroy;

entt::entity g_selected_camera = entt::null;

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

bool is_valid_camera(entt::registry& reg, entt::entity entity) {
	return reg.valid(entity) && reg.has<View2D, Transform2D, Entity_Info>(entity);
}


void do_viewport_gui() {
	Graphics_Context* graphics = get_graphics();
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

			str_t<20> current_res_str = "";
			sprintf(current_res_str, "%ix%i", resolution.x, resolution.y);
			if (ImGui::RBeginCombo("Presets", current_res_str)) {
				ImGui::Text("16:9");
				for (auto res : resolutions_16_9) {
					str_t<20> res_str = "";
					sprintf(res_str, "%ix%i", res.x, res.y);
					if (ImGui::Selectable(res_str, res == resolution)) {
						resolution = res;
					}
				}
				ImGui::Text("16:10");
				for (auto res : resolutions_16_10) {
					str_t<20> res_str = "";
					sprintf(res_str, "%ix%i", res.x, res.y);
					if (ImGui::Selectable(res_str, res == resolution)) {
						resolution = res;
					}
				}
				ImGui::Text("4:3");
				for (auto res : resolutions_4_3) {
					str_t<20> res_str = "";
					sprintf(res_str, "%ix%i", res.x, res.y);
					if (ImGui::Selectable(res_str, res == resolution)) {
						resolution = res;
					}
				}
				ImGui::REndCombo();
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();
		str_ptr_t lbl = is_valid_camera(reg, g_selected_camera)
			? reg.get<Entity_Info>(g_selected_camera).name
			: "No camera in scene";
		if (ImGui::BeginMenu(lbl, is_valid_camera(reg, g_selected_camera))) {
			reg.view<View2D, Transform2D, Entity_Info>().each([&](entt::entity entity, View2D&, Transform2D&, Entity_Info& info){
				if (ImGui::Selectable(info.name, g_selected_camera == entity)) {
					g_selected_camera = entity;
				}
			});
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
		frect view_rect = calculate_view_rect(resolution);
		if (is_valid_camera(reg, g_selected_camera) && view_rect.width > 0 && view_rect.height > 0) {
			auto& view2d = reg.get<View2D>(g_selected_camera);
			graphics_id_t fbo = view2d.render_target;


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

			if (current_resolution != resolution) {
				f32 aspect_ratio = (f32)resolution.width / (f32)resolution.height;
				f32 szx = aspect_ratio * view2d.viewport_size.height;
				f32 szy = view2d.viewport_size.height;
				view2d.viewport_size = {szx, szy};
				view2d.ortho = mz::projection::ortho<float>(-szx / 2.f, szx / 2.f, -szy / 2.f, szy / 2.f, -1000000, 1000000);
			}

			is_game_hovered = ImGui::IsItemHovered();
			bool is_game_focused = ImGui::IsWindowFocused();

			auto gui_wnd_pos = (mz::ivec2)ImGui::GetWindowPos();
			mz::ivec2 wnd_pos = ImGui::GetWindowViewport()->Pos;
			mz::ivec2 raw_mouse_pos = ImGui::GetMousePos() - wnd_pos;
			mz::fvec2 game_pos = (gui_wnd_pos - wnd_pos) + view_rect.pos;
			mz::fvec2 mouse_pos = raw_mouse_pos - game_pos;
			mouse_pos.y = view_rect.size.y - mouse_pos.y;

			fvec2 mouse_ratio = { mouse_pos.x / view_rect.size.x, mouse_pos.y / view_rect.size.y };
			mouse_pos.x = mouse_ratio.x * view2d.viewport_size.x;
			mouse_pos.y = mouse_ratio.y * view2d.viewport_size.y;


			fmat4 cam_transform = reg.get<Transform2D>(g_selected_camera).to_mat4();
			cam_transform.rows[2].w = -.5f;
			fvec2 scale = { cam_transform.rows[0].x, cam_transform.rows[1].y };
			fvec2 view_size = mz::transformation::scale<f32>(scale) * fvec4(view2d.viewport_size, 1, 1);
			fvec2 cam_bot_left = cam_transform.get_translation() - view_size / 2.f;
			fvec2 mouse_world_pos = cam_bot_left + mouse_ratio  * view_size;

			game_input()->state.mouse_pos = mouse_pos;
			game_input()->mouse_world_pos = mouse_world_pos;

			for (int i = AP_KEY_SPACE; i < AP_KEY_COUNT; i++) {
				game_input()->state.keys_press[i] = is_game_focused && ImGui::IsKeyPressed(i, false);
				game_input()->state.keys_down[i] = is_game_focused && ImGui::IsKeyDown(i);
			}

			for (int i = AP_MOUSE_BUTTON_1; i < AP_MOUSE_BUTTON_COUNT; i++) {
				game_input()->state.mouse_press[i] = is_game_focused && ImGui::IsMouseClicked(i);
				game_input()->state.mouse_down[i] = is_game_focused && ImGui::IsMouseDown(i);
			}
		} else {
			ImGui::Text("Add an entity with a View2D and Transform2D for a valid viewport");
		}

	}, flags);
}

extern "C" {
	_export void __cdecl on_load() {
		register_gui_window(&g_viewport);

		auto graphics = get_graphics();
		graphics->set_culling(G_CULL_NONE);
		graphics->set_blending(true);
	}

	_export void __cdecl on_unload() {
		Graphics_Context* graphics = get_graphics();
		for (graphics_id_t render_target : g_render_targets_to_destroy) {
			graphics->destroy_render_target(render_target);
		}
		g_render_targets_to_destroy.clear();
		
		unregister_gui_window(&g_viewport);
	}

	_export void __cdecl on_play_begin() {
		auto& reg = get_entity_registry();
		if (!is_valid_camera(reg, g_selected_camera))  {
			const auto& view = reg.view<View2D, Transform2D>();
			if (view.size() > 0) {
				g_selected_camera = view.front();
			} else {
				log_warn("No camera was found in the scene");
			}
		}

		ImGui::SetWindowFocus("2D Viewport");

		if (is_valid_camera(reg, g_selected_camera)) {
			path_str_t ecs2d_file = "";
			sprintf(ecs2d_file, "%s/%s", get_user_directory(), "2dviewport");

			Binary_Archive archive(ecs2d_file);

			archive.write("selected_camera", reg.get<Entity_Info>(g_selected_camera).name);

			archive.flush();
		}
	}

	_export void __cdecl on_play_stop() {
		Graphics_Context* graphics = get_graphics();
		for (graphics_id_t render_target : g_render_targets_to_destroy) {
			graphics->destroy_render_target(render_target);
		}
		g_render_targets_to_destroy.clear();

		path_str_t ecs2d_file = "";
		sprintf(ecs2d_file, "%s/%s", get_user_directory(), "2dviewport");

		Binary_Archive archive(ecs2d_file);

		if (archive.is_valid_id("selected_camera")) {
			str_ptr_t name = archive.read<name_str_t>("selected_camera");

			auto& reg = get_entity_registry();
			reg.view<View2D, Entity_Info>().each([&name](entt::entity entity, View2D& view, Entity_Info& info) {
				(void)view;
				if (strcmp(info.name, name) == 0) {
					g_selected_camera = entity;
					return;
				}
			});
		}
	}

	_export void __cdecl on_update(float delta) {
		(void)delta;
		
	}

	_export void __cdecl on_render() {

		auto& reg = get_entity_registry();
		auto graphics = get_graphics();

		reg.view<View2D, Transform2D>().each([graphics, &reg](entt::entity entity, View2D& view, Transform2D& view_transform) {
			if (!is_valid_camera(reg, g_selected_camera)) {
				g_selected_camera = entity;
			}

			if (view.render_target == G_NULL_ID) {
				view.render_target = graphics->make_render_target({ 1, 1 });
				g_render_targets_to_destroy.emplace(view.render_target);
				graphics_id_t tex = graphics->get_render_target_texture(view.render_target);
				graphics->set_texture_filtering(tex, G_MIN_FILTER_NEAREST, G_MAG_FILTER_NEAREST);
			}

			graphics->set_clear_color(view.clear_color, view.render_target);
			graphics->clear(G_COLOR_BUFFER_BIT, view.render_target);
		});
	}

	_export void __cdecl on_gui() {
		do_viewport_gui();
	}
}