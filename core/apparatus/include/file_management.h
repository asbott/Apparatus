#pragma once

struct File_Info {
    size_t size;
};

namespace Path {
    AP_API bool read_all_bytes(str_ptr_t path, byte* out_buffer, size_t buffer_size);
    AP_API bool read_as_string(str_ptr_t path, char* out_buffer, size_t buffer_size);

    AP_API bool write_bytes(str_ptr_t path, byte* data, size_t data_size);

    AP_API bool create_directory(str_ptr_t path);

    AP_API bool equals(str_ptr_t path1, str_ptr_t path2);

    AP_API std::error_code remove(str_ptr_t path);

    AP_API bool get_info(str_ptr_t path, File_Info* out_info);

    AP_API bool can_open(str_ptr_t path);

    AP_API bool exists(str_ptr_t path);

    AP_API bool is_file(str_ptr_t path);
    AP_API bool is_directory(str_ptr_t path);

    AP_API void root_name(str_ptr_t path, char* out);

    AP_API void to_absolute(str_ptr_t relative_path, char* out);

    AP_API void to_relative(str_ptr_t path, str_ptr_t base, char* out);

    AP_API std::error_code copy(str_ptr_t src, str_ptr_t dst);

    AP_API void extension_of(str_ptr_t path, char* out);
    AP_API void name_without_extension(str_ptr_t path, char* out);
    AP_API void name_with_extension(str_ptr_t path, char* out);

    AP_API void directory_of(str_ptr_t path, char* out);

    AP_API void iterate_directories(str_ptr_t path, const std::function<void(str_ptr_t)> fn, bool recursive = false);
}