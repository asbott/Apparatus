#pragma once

#include "pch.h"

#include "file_management.h"

#include <filesystem>

namespace fs = std::filesystem;

typedef FILE* File_Handle;

namespace Path {
    bool read_all_bytes(str_ptr_t path, byte* out_buffer, size_t buffer_size) {
        File_Handle file_handle = fopen(path, "r");
        
        if (!file_handle) return false;

        fread(out_buffer, buffer_size, 1, file_handle);

        fclose(file_handle);

        return true;
    }
    bool read_as_string(str_ptr_t path, char* out_buffer, size_t buffer_size) {
        File_Handle file_handle = fopen(path, "r");

        if (!file_handle) return false;

        fgets(out_buffer, (int)buffer_size, file_handle);

        fclose(file_handle);

        return true;
    }

    bool write_bytes(str_ptr_t path, byte* data, size_t data_size) {
        File_Handle file_handle = fopen(path, "w");

        if (!file_handle) return false;

        fwrite(data, data_size, 1, file_handle);

        fclose(file_handle);

        return true;
    }

    bool create_file(str_ptr_t path) {
        std::ofstream ostr(path);
        bool good = ostr.good();
        ostr.close();

        return good;
    }

    bool create_directory(str_ptr_t path) {
        std::error_code err;
        fs::create_directories(path, err);

        return err.value() == 0;
    }

    bool equals(str_ptr_t path1, str_ptr_t path2) {
        return fs::equivalent(path1, path2);
    }

    std::error_code remove(str_ptr_t path) {
        std::error_code err;
        if (is_file(path))
            ::fs::remove(path, err);
        else 
            ::fs::remove_all(path, err);

        return err;
    }

    bool get_info(str_ptr_t path, File_Info* out_info) {
        
        File_Handle file_handle = fopen(path, "r"); 
    
        if (!file_handle) return false;
    
        fseek(file_handle, 0L, SEEK_END); 
    
        out_info->size = ftell(file_handle); 
    
        fclose(file_handle); 
    
        return true; 
    }

    bool can_open(str_ptr_t path) {
        File_Handle file_handle = fopen(path, "r"); 
    
        bool yes = file_handle != NULL;

        if (yes) fclose(file_handle);

        return yes;
    }

    bool exists(str_ptr_t path) {
        return fs::exists(path);
    }

    bool is_file(str_ptr_t path) {
        return fs::is_regular_file(path);
    }
    bool is_directory(str_ptr_t path) {
        return fs::is_directory(path);
    }

    void root_name(str_ptr_t path, char* out) {
        strcpy(out, fs::path(path).root_name().generic_string().c_str());
    }

    void to_absolute(str_ptr_t relative_path, char* out) {
        fs::path fs_abs = fs::absolute(relative_path);
        strcpy(out, fs_abs.generic_string().c_str());
    }

    void to_relative(str_ptr_t path, str_ptr_t base, char* out) {
        fs::path rel = fs::relative(path, base);
        strcpy(out, rel.generic_string().c_str());
    }

    std::error_code copy(str_ptr_t src, str_ptr_t dst) {
        fs::path srcp(src);
        fs::path dstp(dst);

        std::error_code err;
        fs::copy(src, dst, fs::copy_options::overwrite_existing, err);

        return err;
    }

    void extension_of(str_ptr_t path, char* out) {
        size_t len = strlen(path);
        for (s64 i = (s64)len - 1LL; i >= 0LL; i--) {
            if (path[i] == '.') {
                strcpy(out, &path[i + 1]);
                return;
            }

            if (path[i] == '/' || path[i] == '\\') {
                break;
            }
        }
        strcpy(out, "");
    }
    void name_without_extension(str_ptr_t path, char* out) {
        if (path[0] == '.') {
            strcpy(out, "");
            return;
        }
        size_t len = strlen(path);
        size_t end = len;
        for (s64 i = (s64)len - 1LL; i >= 0; i--) {
            if (path[i] == '.') {
                end = i;
            }
            if (path[i] == '\\' || path[i] == '/' || i == 0) {
                memcpy(out, &path[i + 1], end - i - 1);
                out[end] = '\0';
                return;
            }
        }
    }
    void name_with_extension(str_ptr_t path, char* out) {

        char prefix[MAX_PATH_LENGTH] = "";
        name_without_extension(path, prefix);

        char suffix[MAX_PATH_LENGTH] = "";
        extension_of(path, suffix);

        if (strlen(suffix) > 0)
            sprintf(out, "%s.%s", prefix, suffix);
        else 
            strcpy(out, prefix);
    }
    bool has_extension(str_ptr_t path) {
        return fs::path(path).has_extension();
    }
    void directory_of(str_ptr_t path, char* out) {
        if (path[0] == '.') {
            strcpy(out, "");
            return;
        }
        size_t len = strlen(path);
        for (s64 i = (s64)len - 1LL; i >= 0LL; i--) {
            if (path[i] == '\\' || path[i] == '/') {
                memcpy(out, path, i + 1);
                out[i] = '\0';
                break;
            }
        }
    }
    void iterate_directories(str_ptr_t path, const std::function<void(str_ptr_t)> fn, bool recursive) {
        if (!Path::exists(path)) return;
        if (recursive) {
            fs::recursive_directory_iterator it(path);
            for (auto entry : it) {
                if (entry.is_regular_file() || entry.is_directory())
                fn(entry.path().generic_string().c_str());
            }
        } else {
            fs::directory_iterator it(path);
            for (auto entry : it) {
                if (entry.is_regular_file() || entry.is_directory())
                fn(entry.path().generic_string().c_str());
            }
        }
    }
}