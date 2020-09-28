#pragma once

#include "apparatus.h"

#include "asset_manager/asset_manager.h"

tag(component, custom_gui)
struct Transform2D {
	fmat4 matrix = fmat4(1);
};



tag(component)
struct Sprite2D {

	tag(property, texture)
	asset_id_t texture = G_NULL_ID;

	tag(property, color)
	color tint = COLOR_WHITE;
};

tag(component)
struct View2D {
	tag(property, color)
	color16 clear_color = color16(.3f, .1f, .4f, 1.f);
};

inline void on_gui(Transform2D* transform, ImGuiContext* ctx) {
	ImGui::SetCurrentContext(ctx);

	auto& matrix = transform->matrix;

	fvec2 pos (
		matrix.rows[0].w,
		matrix.rows[1].w
	);
	ImGui::DragFloat2("position", pos.ptr);
	matrix.rows[0].w = pos.x;
	matrix.rows[1].w = pos.y;

	/*float rotation = atan(matrix.rows[0].y / matrix.rows[1].y);
	matrix.rotate(-rotation, fvec3(0, 0, 1));
	ImGui::DragFloat("rotation", &rotation, .05f);
	matrix.rotate(rotation, fvec3(0, 0, 1));

	fvec2 scale (
	sqrt(matrix.rows[0].x * matrix.rows[0].x + matrix.rows[0].y * matrix.rows[0].y), 
	sqrt(matrix.rows[1].x * matrix.rows[1].x + matrix.rows[1].y * matrix.rows[1].y)
	);
	matrix.scale((-scale));
	ImGui::DragFloat2("scale", scale.ptr, .01f);
	matrix.scale(scale);*/
}