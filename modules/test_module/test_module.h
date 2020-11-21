#pragma once

#include "apparatus.h"

#include "asset_manager/asset_manager.h"

module_scope {

	tag(component)
	struct BallMovement {
		tag(property)
		f32 hforce = 13000;
		tag(property)
		f32 jump_force = 10000;
	};

	tag(component)
	struct WASDMovement {
		tag(property)
		float move_speed = 1000.f;
	};
	
	tag(component)
	struct AnimatedWASDMovement {
		tag(property)
		float hspeed = 500.f;
		tag(property)
		float vspeed = 500.f;

		tag(property, asset(SpriteAnimation2DPreset))
		asset_id_t walk_right = NULL_ASSET_ID;

		tag(property, asset(SpriteAnimation2DPreset))
		asset_id_t walk_up = NULL_ASSET_ID;

		tag(property, asset(SpriteAnimation2DPreset))
		asset_id_t walk_down = NULL_ASSET_ID;
	};

	tag(component)
	struct FollowEntity {
		tag(entity, property)
		entity_t target;
	};
}
