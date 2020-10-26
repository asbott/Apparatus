#pragma once

#include "asset_manager/asset_manager.h"

#include "apparatus.h"

struct Sprite_Animation_2D_Preset_Data {
	asset_id_t texture_sheet = NULL_ASSET_ID;
    f32 frames_per_second = 14;
	irange target_frames = {0, 1};
};

AP_NS_BEGIN(asset)
AP_NS_BEGIN(sprite_animation_2d_preset)

struct Params {
    asset_id_t texture_sheet = NULL_ASSET_ID;

    f32 frames_per_second = 14;
	irange target_frames = {0, 1};

	s32 preview_frame = 0;
	f32 preview_time = 0;
};

constexpr size_t param_size = sizeof(Params);
constexpr size_t runtime_data_size = sizeof(Sprite_Animation_2D_Preset_Data);
constexpr Icon_Type icon = ICON_TYPE_FILE;
constexpr char name[] = "SpriteAnimation2DPreset";
inline const std::initializer_list<Dynamic_String> extensions = { "sa2d" };

size_t tell_size(str_ptr_t path, void* parameter);
byte* load(byte* stream, str_ptr_t path, void* parameter);

void unload(byte* stream);

void on_gui(void* parameter);

void set_default_params(void* parameter);

AP_NS_END(asset)
AP_NS_END(sprite_animation_2d_preset)

