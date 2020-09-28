#pragma once

AP_API byte* load_image_from_memory(const byte* encoded, s32 size, s32* out_width, s32* out_height, s32* out_channel_count, s32 desired_channels = 0);
AP_API byte* load_image_from_file(const char* path, s32* out_width, s32* out_height, s32* out_channel_count, s32 desired_channels = 0);
AP_API byte* write_image_to_png_in_memory(const byte* pixels, s32 width, s32 height, s32 channels, s32* out_length);

AP_API const char* get_failure_reason();

AP_API void set_vertical_flip_on_load(bool value);
AP_API void set_vertical_flip_on_write(bool value);

AP_API void free_image(byte* image);