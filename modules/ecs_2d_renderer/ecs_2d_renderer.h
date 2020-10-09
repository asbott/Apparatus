#pragma once

#include "apparatus.h"

#include "asset_manager/asset_manager.h"

tag(component)
struct Transform2D {
	tag(property)
	fvec2 position;
	tag(property)
	f32 rotation;
	tag(property)
	fvec2 scale = fvec2(1);

	fmat4 to_mat4() {
		fmat4 m = mz::transformation::translation<f32>(position);
		m.rotate(rotation, { 0, 0, -1 });
		m.scale(scale - fvec2(1));
		return m;
	}
};

tag(component)
struct Sprite2D {

	tag(property, texture)
	asset_id_t texture = G_NULL_ID;

	tag(property, color)
	color tint = COLOR_WHITE;

	tag(property)
	fvec2 origin = 0;
};

tag(component)
struct View2D {
	View2D() {
		render_target = G_NULL_ID;
	}
	tag(property, color)
	color16 clear_color = color16(.3f, .1f, .4f, 1.f);

	graphics_id_t render_target = G_NULL_ID;
};