#pragma once

#include "pch.h"

#include "file_management.h"

#include <filesystem>

namespace fs = std::filesystem;

typedef FILE* File_Handle;

namespace Path {
    bool read_all_bytes(const char* path, byte* out_buffer, size_t buffer_size) {
        File_Handle file_handle = fopen(path, "r");
        
        if (!file_handle) return false;

        fread(out_buffer, buffer_size, 1, file_handle);

        fclose(file_handle);

        return true;
    }
    bool read_as_string(const char* path, char* out_buffer, size_t buffer_size) {
        File_Handle file_handle = fopen(path, "r");

        if (!file_handle) return false;

        fgets(out_buffer, (int)buffer_size, file_handle);

        fclose(file_handle);

        return true;
    }

    bool write_bytes(const char* path, byte* data, size_t data_size) {
        File_Handle file_handle = fopen(path, "w");

        if (!file_handle) return false;

        fwrite(data, data_size, 1, file_handle);

        fclose(file_handle);

        return true;
    }

    std::error_code remove(const char* path) {
        std::error_code err;
        ::fs::remove(path, err);

        return err;
    }

    bool get_info(const char* path, File_Info* out_info) {
        
        File_Handle file_handle = fopen(path, "r"); 
    
        if (!file_handle) return false;
    
        fseek(file_handle, 0L, SEEK_END); 
    
        out_info->size = ftell(file_handle); 
    
        fclose(file_handle); 
    
        return true; 
    }

    bool can_open(const char* path) {
        File_Handle file_handle = fopen(path, "r"); 
    
        bool yes = file_handle != NULL;

        if (yes) fclose(file_handle);

        return yes;
    }

    void to_absolute(const char* relative_path, char* out) {
        fs::path fs_abs = fs::absolute(relative_path);

        strcpy(out, fs_abs.generic_string().c_str());
    }

    std::error_code copy(const char* src, const char* dst) {
        fs::path srcp(src);
        fs::path dstp(dst);

        std::error_code err;
        fs::copy(src, dst, fs::copy_options::overwrite_existing, err);

        return err;
    }

    void extension_of(const char* path, char* out) {
        size_t len = strlen(path);
        for (s64 i = (s64)len - 1LL; i >= 0LL; i--) {
            if (path[i] == '.') {
                strcpy(out, &path[i + 1]);
                break;
            }
        }
    }
    void name_without_extension(const char* path, char* out) {
        if (path[0] == '.') {
            strcpy(out, "");
            return;
        }
        size_t len = strlen(path);
        size_t end = len - 1;
        for (s64 i = (s64)len - 1LL; i >= 0; i--) {
            if (path[i] == '.') {
                end = i;
            }
            if (path[i] == '\\' || path[i] == '/') {
                memcpy(out, &path[i + 1], end - i - 1);
                out[end] = '\0';
                break;
            }
        }
    }
    void name_with_extension(const char* path, char* out) {

        char prefix[MAX_PATH_LENGTH] = "";
        name_without_extension(path, prefix);

        char suffix[MAX_PATH_LENGTH] = "";
        extension_of(path, suffix);

        sprintf(out, "%s.%s", prefix, suffix);
    }
    void directory_of(const char* path, char* out) {
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
}