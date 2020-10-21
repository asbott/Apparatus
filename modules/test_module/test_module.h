#pragma once

#include "apparatus.h"

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
	
}
