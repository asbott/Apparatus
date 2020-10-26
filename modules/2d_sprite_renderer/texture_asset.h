#pragma once

#include "apparatus.h"

struct Texture_Data {
	graphics_id_t graphics_id;
	ivec2 size;
	s32 channels;
	void* data;
};

AP_NS_BEGIN(asset)
AP_NS_BEGIN(texture)

struct Params {
    s32 nchannels = 4;
    graphics_enum_t min_filter = G_MIN_FILTER_LINEAR;
    graphics_enum_t mag_filter = G_MAG_FILTER_NEAREST;
    graphics_enum_t wrap_mode = G_WRAP_CLAMP_TO_BORDER;
};

constexpr size_t param_size = sizeof(Params);
constexpr size_t runtime_data_size = sizeof(Texture_Data);
constexpr Icon_Type icon = ICON_TYPE_TEXTURE;
constexpr char name[] = "Texture";
inline const std::initializer_list<Dynamic_String> extensions = { "png", "jpg", "jpeg", "bmp", "psd" };

size_t tell_size(str_ptr_t path, void* parameter);
byte* load(byte* stream, str_ptr_t path, void* parameter);
void unload(byte* stream);

void on_gui(void* parameter);

void set_default_params(void* parameter);

AP_NS_END(asset)
AP_NS_END(texture)

