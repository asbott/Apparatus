#include "apparatus.h"

#include "test_module.h"

#include "2d_sprite_renderer/2d_sprite_renderer.h"
#include "2d_physics/2d_physics.h"
#include "2d_viewport/2d_viewport.h"

module_scope {

	module_function(void) on_load() {
		
	}

    module_function(void) on_unload() {
		
    }

	module_function(void) on_play_begin() {
		
	}

	module_function(void) on_play_end() {
		
	}

    module_function(void) on_update(float delta) {
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

		reg.view<Transform2D, AnimatedWASDMovement, SpriteAnimation2D>().each([delta](Transform2D& transform, AnimatedWASDMovement& movement, SpriteAnimation2D& anim) {
			fvec2 dir = 0;
			if (game_input()->is_key_down(AP_KEY_W)) {
				dir = fvec2(0, 1);
			}
			if (game_input()->is_key_down(AP_KEY_A)) {
				anim.xflip = true;
				dir = fvec2(-1, 0);
			}
			if (game_input()->is_key_down(AP_KEY_S)) {
				dir = fvec2(0, -1);
			}
			if (game_input()->is_key_down(AP_KEY_D)) {
				anim.xflip = false;
				dir = fvec2(1, 0);
			}

			if (dir.x == 1 || dir.x == -1) {
				anim.animation_preset = movement.walk_right;
			} else if (dir.y == 1) {
				anim.animation_preset = movement.walk_up;
			} else if (dir.y == -1) {
				anim.animation_preset = movement.walk_down;
			}

			transform.position += dir * fvec2(movement.hspeed * delta, movement.vspeed * delta);
		});

		reg.view<FollowEntity, Transform2D>().each([&reg](FollowEntity& follow, Transform2D& transform) {
			if (!reg.valid(follow.target)) return;

			auto& target_transform = reg.get<Transform2D>(follow.target);
			transform.position = target_transform.position;
		});
    }

	module_function(void) on_render() {
		
	}

	module_function(void) on_gui() {
		
	}
}