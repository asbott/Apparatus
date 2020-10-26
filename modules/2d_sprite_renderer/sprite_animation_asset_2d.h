#pragma once

#include "asset_manager/asset_manager.h"

#include "apparatus.h"

struct Texture_Sheet_Runtime_Data {
    asset_id_t texture = NULL_ASSET_ID;
    fvec2 cell_size = 0;
    s32 nempty = 0;
};

AP_NS_BEGIN(asset)
AP_NS_BEGIN(texture_sheet_2d)

struct Params {
    asset_id_t texture = NULL_ASSET_ID;
    fvec2 cell_size = 32;
};

constexpr size_t param_size = sizeof(Params);
constexpr size_t runtime_data_size = sizeof(Texture_Sheet_Runtime_Data);
constexpr Icon_Type icon = ICON_TYPE_FILE;
constexpr char name[] = "TextureSheet2D";
inline const std::initializer_list<Dynamic_String> extensions = { "sh2d" };

size_t tell_size(str_ptr_t path, void* parameter);
byte* load(byte* stream, str_ptr_t path, void* parameter);

void unload(byte* stream);

void on_gui(void* parameter);

void set_default_params(void* parameter);

AP_NS_END(asset)
AP_NS_END(texture_sheet_2d)

