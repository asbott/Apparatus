#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "image_import.h"

byte* load_image_from_memory(const byte* encoded, s32 size, s32* out_width, s32* out_height, s32* out_channel_count, s32 desired_channels) {
    return stbi_load_from_memory(encoded, size, out_width, out_height, out_channel_count, desired_channels);
}

byte* load_image_from_file(const char* path, s32* out_width, s32* out_height, s32* out_channel_count, s32 desired_channels) {
    return stbi_load(path, out_width, out_height, out_channel_count, desired_channels);
}

byte* write_image_to_png_in_memory(const byte* pixels, s32 width, s32 height, s32 channels, s32* out_length) {
    return stbi_write_png_to_mem(pixels, 0, width, height, channels, out_length);
}

const char* get_failure_reason() {
    return stbi_failure_reason();
}

void set_vertical_flip_on_load(bool value) {
    stbi_set_flip_vertically_on_load(value);
}
void set_vertical_flip_on_write(bool value) {
    stbi_flip_vertically_on_write(value);
}

void free_image(byte* image) {
    stbi_image_free(image);
}