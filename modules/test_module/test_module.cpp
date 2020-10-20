#include "apparatus.h"

#include "test_module.h"

#include "2d_sprite_renderer/2d_sprite_renderer.h"
#include "2d_physics/2d_physics.h"
#include "2d_viewport/2d_viewport.h"

extern "C" {

	_export void __cdecl on_load() {
		
	}

    _export void __cdecl on_unload() {
		
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

		reg.view<WASDMovement, Transform2D>().each([delta](WASDMovement& movement, Transform2D& transform) {
			if (game_input()->is_key_down(AP_KEY_W)) {
				transform.position += fvec2(0, 1) * movement.move_speed * delta;
			}
			if (game_input()->is_key_down(AP_KEY_A)) {
				transform.position += fvec2(-1, 0) * movement.move_speed * delta;
			}
			if (game_input()->is_key_down(AP_KEY_S)) {
				transform.position += fvec2(0, -1) * movement.move_speed * delta;
			}
			if (game_input()->is_key_down(AP_KEY_D)) {
				transform.position += fvec2(1, 0) * movement.move_speed * delta;
			}
		});
    }

	_export void __cdecl on_render() {
		
	}

	_export void __cdecl on_gui() {
		
	}
}