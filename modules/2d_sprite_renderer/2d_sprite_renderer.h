#pragma once

#include "apparatus.h"

#include "asset_manager/asset_manager.h"



tag(component)
struct Sprite2D {

	tag(property, asset("Texture"))
	asset_id_t texture = G_NULL_ID;

	tag(property, color)
	color tint = COLOR_WHITE;

	tag(property)
	fvec2 origin = 0;

	tag(property)
	int depth_level = 0;
};

tag(component)
struct SpriteAnimation2D {
	tag(property)
	bool use_asset = false;

	tag(property, asset("Sprite Animation 2D"))
	asset_id_t anim_asset;
};

struct Texture_Data {
	graphics_id_t graphics_id;
	ivec2 size;
	s32 channels;
	void* data;
};