#pragma once

#define tag(...)

tag(component, nut)
struct Transform {
	tag(property)
	fvec3 position;
	tag(property)
	fvec2 scale;
	tag(property)
	fvec4 color;
};

tag(component)
struct SpriteComponent {
	float size;
};

tag(component)
struct SpeedComponent {
	tag(property)
	float speed;
};