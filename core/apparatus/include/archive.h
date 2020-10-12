#pragma once

typedef std::function<bool(str_ptr_t)> archive_iterate_t;

struct Binary_Archive {
    struct Entry {
        index_t index_in_buffer;
        size_t sz;
    };
    AP_API Binary_Archive(str_ptr_t file_path, size_t hint_size = (size_t)0);
    AP_API ~Binary_Archive();

    AP_API void hint_size(size_t sz) { _byte_buffer.reserve(sz); }

    AP_API void write(str_ptr_t id, const void* val, size_t sz);

    AP_API void* read(str_ptr_t id, size_t* size_out = NULL);

    template <typename type_t>
    inline void write(str_ptr_t id, const type_t& item) {
        write(id, &item, sizeof(type_t));
    }
    template <typename type_t>
    inline type_t& read(str_ptr_t id) {
        size_t sz = 0;
        type_t& ret = *(type_t*)read(id, &sz);
        ap_assert(sz >= sizeof(type_t), "Invalid read type '{}' with size {} (expected {})", typeid(type_t).name(), sizeof(type_t), sz);
        return ret;
    }

    AP_API bool is_valid_id(str_ptr_t id);

    AP_API size_t size_of(str_ptr_t id);

    AP_API void flush();

    inline void iterate(const archive_iterate_t& fn) {
        for (auto& [id, entry] : _str_id_to_entry_map) {
            if (!fn(id.c_str())) break;
        }
    }

    Dynamic_Array<byte> _byte_buffer;
    Ordered_Map<Dynamic_String, Entry> _str_id_to_entry_map;
    bool _should_flush_on_destruction = false;
    str_ptr_t file_path;
};