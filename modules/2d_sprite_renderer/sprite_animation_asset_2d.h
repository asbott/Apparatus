#pragma once

#include "2d_sprite_renderer.h"

AP_NS_BEGIN(asset)
AP_NS_BEGIN(texture_sheet_2d)

struct Params {
    asset_id_t texture = NULL_ASSET_ID;
    fvec2 cell_size = 0;
    s32 nempty = 0;
};

constexpr size_t param_size = sizeof(Params);
constexpr Icon_Type icon = ICON_TYPE_FILE;
constexpr char name[] = "Texture Sheet 2D";
inline const Hash_Set<Dynamic_String> extensions = { "sh2d" };

size_t tell_size(str_ptr_t path, void* parameter);
byte* load(byte* stream, str_ptr_t path, void* parameter);

void unload(byte* stream);

void on_gui(void* parameter);

void set_default_params(void* parameter);

struct Texture_Sheet_Runtime_Data {
    graphics_id_t preview_render_target = G_NULL_ID;
    mz::fmat4 ortho;
    mz::fmat4 view = 1;
};

AP_NS_END(asset)
AP_NS_END(texture_sheet_2d)