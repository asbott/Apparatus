#include "texture_asset.h"

#include "asset_manager/asset_manager.h"

AP_NS_BEGIN(asset)
AP_NS_BEGIN(texture)

size_t tell_size(str_ptr_t path, void* parameter) {
    Params& params = *(Params*)parameter;
    ivec3 sz;
    auto* img = load_image_from_file(path, &sz.x, &sz.y, &sz.z, params.nchannels);

    if (img) free_image(img);
    else return 0;

    return sz.x * sz.y * params.nchannels + sizeof(Texture_Data);
}
byte* load(byte* stream, str_ptr_t path, void* parameter) {
    Params& params = *(Params*)parameter;
    auto graphics = get_graphics();
    Texture_Data tex;

    tex.graphics_id = graphics->make_texture(G_BUFFER_USAGE_STATIC_WRITE);
    auto* img = load_image_from_file(path, &tex.size.x, &tex.size.y, &tex.channels, params.nchannels);

    tex.channels = params.nchannels;

    if (!img) {
        log_error("Failed loading texture from: \n{}\nReason: ", get_failure_reason());
        return NULL;
    }

    graphics->set_texture_filtering(tex.graphics_id, params.min_filter, params.mag_filter);
    graphics->set_texture_wrapping(tex.graphics_id, params.wrap_mode);

    size_t img_size = tex.size.width * tex.size.height * tex.channels;

    tex.data = stream + sizeof(Texture_Data);

    memcpy(stream, &tex, sizeof(Texture_Data));
    memcpy(stream + sizeof(Texture_Data), img, img_size);

    graphics_enum_t fmt = G_TEXTURE_FORMAT_RGBA;
    switch (tex.channels) {
        case 1: G_TEXTURE_FORMAT_RED;  break;
        case 2: G_TEXTURE_FORMAT_RG;   break;
        case 3: G_TEXTURE_FORMAT_RGB;  break;
        case 4: G_TEXTURE_FORMAT_RGBA; break;
    }
    graphics->set_texture_data(tex.graphics_id, (byte*)tex.data, tex.size, fmt);

    free_image(img);

    return stream + img_size + sizeof(Texture_Data);
}
void unload(byte* stream) {
    Texture_Data& tex = *(Texture_Data*)stream;

    get_graphics()->destroy_texture(tex.graphics_id);
}

void on_gui(void* parameter) {
    Params& params = *(Params*)parameter;

    auto graphics_option = [&params](graphics_enum_t& to_be_set, graphics_enum_t x) {
        if (ImGui::Selectable(get_graphics_enum_string(x), x == to_be_set)) {
            to_be_set = x;
        }
    };

    if (ImGui::RBeginCombo("Min filter", get_graphics_enum_string(params.min_filter))) {

        graphics_option(params.min_filter, G_MIN_FILTER_NEAREST);
        graphics_option(params.min_filter, G_MIN_FILTER_LINEAR);
        graphics_option(params.min_filter, G_MIN_FILTER_NEAREST_MIPMAP_LINEAR);
        graphics_option(params.min_filter, G_MIN_FILTER_NEAREST_MIPMAP_NEAREST);
        graphics_option(params.min_filter, G_MIN_FILTER_LINEAR_MIPMAP_LINEAR);
        graphics_option(params.min_filter, G_MIN_FILTER_LINEAR_MIPMAP_NEAREST);

        ImGui::REndCombo();
    }

    if (ImGui::RBeginCombo("Mag filter", get_graphics_enum_string(params.mag_filter))) {

        graphics_option(params.mag_filter, G_MAG_FILTER_NEAREST);
        graphics_option(params.mag_filter, G_MAG_FILTER_LINEAR);

        ImGui::REndCombo();
    }

    if (ImGui::RBeginCombo("Wrap mode", get_graphics_enum_string(params.wrap_mode))) {

        graphics_option(params.wrap_mode, G_WRAP_CLAMP_TO_BORDER);
        graphics_option(params.wrap_mode, G_WRAP_CLAMP_TO_EDGE);
        graphics_option(params.wrap_mode, G_WRAP_MIRRORED_REPEAT);
        graphics_option(params.wrap_mode, G_WRAP_REPEAT);

        ImGui::REndCombo();
    }
}

void set_default_params(void* parameter) {
    *(Params*)parameter = Params();
}



AP_NS_END(asset)
AP_NS_END(texture)