#include "apparatus.h"

#include "test.h"

#include "ecs_2d_renderer/ecs_2d_renderer.h"

void* wnd;
Windows_Context* windows;

extern "C" {
	_export void __cdecl on_load(Graphics_Context* graphics) {
		log_info("on_load called in test_module!");
		(void)graphics;
        
		windows = graphics->get_windows_context();
		wnd = windows->main_window_handle;
	}

    _export void __cdecl on_unload(Graphics_Context* graphics) {
		(void)graphics;
        
    }

    _export void __cdecl on_update(float delta) {
		(void)delta;

		auto& reg = get_entity_registry();

		reg.view<Transform2D, KeyboardMovement>().each([delta](Transform2D& transform, KeyboardMovement& move) {
			if (game_input()->is_key_down(AP_KEY_W)) {
				transform.position += (fvec2(0, move.vspeed) * delta);
			}	
			if (game_input()->is_key_down(AP_KEY_A)) {
				transform.position += (fvec2(-move.hspeed, 0) * delta);
			}
			if (game_input()->is_key_down(AP_KEY_S)) {
				transform.position += (fvec2(0, -move.vspeed) * delta);
			}
			if (game_input()->is_key_down(AP_KEY_D)) {
				transform.position += (fvec2(move.hspeed, 0) * delta);
			}

			if (game_input()->is_mouse_pressed(AP_MOUSE_BUTTON_LEFT)) {
				transform.position = (game_input()->mouse_world_pos);
			}
		});
    }

	_export void __cdecl on_render(Graphics_Context* graphics) {
		(void)graphics;
		if (windows->should_close(wnd)) {
			quit();
		}
	}

	_export void __cdecl on_gui(Graphics_Context* graphics, ImGuiContext* imgui_ctx) {
		(void)graphics;(void)imgui_ctx;
	}
}