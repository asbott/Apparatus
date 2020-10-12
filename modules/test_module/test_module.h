#pragma once

#include "apparatus.h"

tag(component)
struct BallMovement {
	tag(property)
	f32 hforce = 13000;
	tag(property)
	f32 jump_force = 10000;
};