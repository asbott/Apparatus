#pragma once

struct File_Info {
    size_t size;
};

namespace Path {
    bool read_all_bytes(const char* path, byte* out_buffer, size_t buffer_size);
    bool read_as_string(const char* path, char* out_buffer, size_t buffer_size);

    bool write_bytes(const char* path, byte* data, size_t data_size);

    std::error_code remove(const char* path);

    bool get_info(const char* path, File_Info* out_info);

    bool can_open(const char* path);

    void to_absolute(const char* relative_path, char* out);

    std::error_code copy(const char* src, const char* dst);

    void extension_of(const char* path, char* out);
    void name_without_extension(const char* path, char* out);
    void name_with_extension(const char* path, char* out);

    void directory_of(const char* path, char* out);
}