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

    _export void __cdecl on_update(float delta) {
		(void)delta;

		auto& reg = get_entity_registry();

		// View all "entities" with a movement and physics body component
		auto view = reg.view<WASDMovement, PhysicsBody2D>();

		// Iterate all "entities" in the view
		view.each([](WASDMovement& movement, PhysicsBody2D& body) {


			if (game_input()->is_key_down(AP_KEY_A)) {
				body.velocity.x = -movement.hspeed; // When A is HELD down, move left
			} else if (game_input()->is_key_down(AP_KEY_D)) {
				body.velocity.x = movement.hspeed;  // When D is HELD down, move right
			} else {
				body.velocity.x = 0;
			}

			if (game_input()->is_key_pressed(AP_KEY_W)) {
				body.velocity.y += movement.jump_force; // When W is PRESSED (was up last frame, down now) apply a jump force
			}
		});
    }

	_export void __cdecl on_render(Graphics_Context* graphics) {
		(void)graphics;
	}

	_export void __cdecl on_gui(Graphics_Context* graphics, ImGuiContext* imgui_ctx) {
		(void)graphics;(void)imgui_ctx;
	}
}