#include "apparatus.h"

#include "test_module.h"

#include "ecs_2d_renderer/ecs_2d_renderer.h"
#include "2d_physics/2d_physics.h"

extern "C" {
	_export void __cdecl on_load(Graphics_Context* graphics) {
		(void)graphics;
	}

    _export void __cdecl on_unload(Graphics_Context* graphics) {
		(void)graphics;
    }

	_export void __cdecl on_play_begin() {
		
	}

	_export void __cdecl on_play_end() {
		
	}

    _export void __cdecl on_update(float delta) {
		(void)delta;

		auto& reg = get_entity_registry();

		auto view = reg.view<BallMovement, PhysicsBody2D>();

		view.each([delta, &reg](BallMovement& movement, PhysicsBody2D& body) {
			if (game_input()->is_key_down(AP_KEY_A)) {
				body.apply_force(fvec2(-movement.hforce * delta, 0));
			} else if (game_input()->is_key_down(AP_KEY_D)) {
				body.apply_force(fvec2(movement.hforce * delta, 0));
			}

			if (game_input()->is_key_pressed(AP_KEY_W)) {
				body.apply_force(fvec2(0, movement.jump_force));
			}
		});
    }

	_export void __cdecl on_render(Graphics_Context* graphics) {
		(void)graphics;
	}

	_export void __cdecl on_gui(Graphics_Context* graphics) {
		(void)graphics;
	}
}