#pragma once

#include "apparatus.h"

tag(component)
struct View2D {
	View2D() {
		render_target = G_NULL_ID;
	}
	tag(property, color)
		color16 clear_color = color16(.3f, .1f, .4f, 1.f);

	graphics_id_t render_target = G_NULL_ID;

	fmat4 ortho;
	fvec2 viewport_size = fvec2(1600, 900);
};

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
