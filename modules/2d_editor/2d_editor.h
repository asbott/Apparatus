#pragma once

#include "apparatus.h"

struct Editor_View {
	fmat4 transform;
	fmat4 ortho;
	graphics_id_t render_target;
	fcolor16 clear_color = color16(.3f, .1f, .4f, 1.f);
};

struct Editor_2D_Function_Library {
	typedef Editor_View*(*get_view_t)();
	get_view_t get_view;
};