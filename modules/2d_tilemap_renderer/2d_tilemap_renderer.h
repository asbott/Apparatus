#pragma once

#include "apparatus.h"

#include "asset_manager/asset_manager.h"

tag(component)
struct TileMap2D {
    tag(property, asset("Texture"))
    asset_id_t source_texture;

    tag(property)
    fvec2 tile_size;

    graphics_id_t render_target = G_NULL_ID;
};