#include "pch.h"

#include "archive.h"

constexpr char HEADER_TOKEN[] = "\1\2\3\4\5\6\7\10apparatus\11\12\13\14\15archive\n";
constexpr char ITEM_ID_TOKEN[] = "\n$!$!£\7\1\3\12\15\35\31";
constexpr char ITEM_SIZE_BEGIN_TOKEN[] = "\"\"";
constexpr char ITEM_SIZE_END_TOKEN[] = "\"";


Binary_Archive::Binary_Archive(str_ptr_t file_path, size_t hint_size) : file_path(file_path) {
    if (hint_size) _byte_buffer.reserve(hint_size);

    if (Path::can_open(file_path)) {
        File_Info finfo;
        Path::get_info(file_path, &finfo);

        byte* buffer_head = (byte*)malloc(finfo.size);
        Path::read_all_bytes(file_path, buffer_head, finfo.size);

        byte* buffer_ptr = buffer_head;
        for (int i = 0; i < finfo.size; i++) {
            if (buffer_head[i] == '\r\n') buffer_head[i] = '\n';
        }

        if (memcmp(buffer_ptr, HEADER_TOKEN, sizeof(HEADER_TOKEN)-1) != 0) return;

        buffer_ptr += sizeof(HEADER_TOKEN)-1;

        int mode = 0;

        Dynamic_Array<byte> word;
        byte* id_pos = NULL;
        byte* id_end_pos = NULL;
        byte* sz_pos = NULL;
        byte* sz_end_pos = NULL;
        byte* item_pos = NULL;
        for (; buffer_ptr < buffer_head + finfo.size; buffer_ptr++) {
            if (mode == 0 && memcmp(buffer_ptr, ITEM_ID_TOKEN, sizeof(ITEM_ID_TOKEN)-1) == 0) {
                buffer_ptr +=  sizeof(ITEM_ID_TOKEN)-1;
                id_pos = buffer_ptr;
                mode++;
            }

            if (mode == 1) {
                if (memcmp(buffer_ptr, ITEM_SIZE_BEGIN_TOKEN, sizeof(ITEM_SIZE_BEGIN_TOKEN)-1) == 0) {
                    id_end_pos = buffer_ptr;
                    buffer_ptr += sizeof(ITEM_SIZE_BEGIN_TOKEN)-1;
                    sz_pos = buffer_ptr;
                    mode++;
                    word.clear();
                } else {
                    word.push_back(buffer_ptr[0]);
                }
            }

            if (mode == 2) {
                if (memcmp(buffer_ptr, ITEM_SIZE_END_TOKEN, sizeof(ITEM_SIZE_END_TOKEN)-1) == 0) {
                    sz_end_pos = buffer_ptr;
                    buffer_ptr += sizeof(ITEM_SIZE_END_TOKEN)-1;
                    item_pos = buffer_ptr;
                    mode++;
                    word.clear();
                } else {
                    word.push_back(buffer_ptr[0]);
                }
            }

            if (mode == 3) {
                size_t size_nbytes = sz_end_pos - sz_pos;
                str_t<sizeof(size_t) + 1> size_str; memset(size_str, 0, sizeof(size_str));
                memcpy(size_str, sz_pos, size_nbytes);
                size_t item_size = atoll(size_str);
                if (buffer_ptr == item_pos + item_size) {
                    Dynamic_String id((char*)id_pos, (char*)id_end_pos);
                    write(id.c_str(), item_pos, item_size);
                    mode = 0;
                    word.clear();
                    id_pos = NULL;
                    id_end_pos = NULL;
                    sz_pos = NULL;
                    sz_end_pos = NULL;
                    item_pos = NULL;
                    buffer_ptr--;
                } else {
                    word.push_back(buffer_ptr[0]);
                }
            }
        }
    }
}
Binary_Archive::~Binary_Archive() {
    if (_should_flush_on_destruction) {
        flush();
    }
}

void Binary_Archive::write(str_ptr_t id, const void* val, size_t sz) {
    if (_str_id_to_entry_map.count(id) == 0) {
        _str_id_to_entry_map[id] = { _byte_buffer.size(), sz };
        _byte_buffer.resize(_byte_buffer.size() + sz);
    }
    ap_assert(sz == size_of(id), "Size mismatch. Expected {}, got {}", size_of(id), sz);

    byte* dst = &_byte_buffer[_str_id_to_entry_map[id].index_in_buffer];
    memcpy(dst, val, sz);

    _should_flush_on_destruction = true;
}

void* Binary_Archive::read(str_ptr_t id, size_t* size_out) {
    if (!is_valid_id(id)) return NULL;
    if (size_out) *size_out = size_of(id);

    return &_byte_buffer[_str_id_to_entry_map[id].index_in_buffer];
}

bool Binary_Archive::is_valid_id(str_ptr_t id) {
    return _str_id_to_entry_map.count(id) > 0;
}

size_t Binary_Archive::size_of(str_ptr_t id) {  
    if (_str_id_to_entry_map.count(id) == 0) return 0;
    return _str_id_to_entry_map[id].sz;
}

void Binary_Archive::flush() {
    _should_flush_on_destruction = false;
    std::ofstream ostr;
    ostr.open(file_path);

    ostr << HEADER_TOKEN;

    for (auto& [str, entry] : _str_id_to_entry_map) {
        auto id = str;

        size_t sz = size_of(id.c_str());
        ostr << ITEM_ID_TOKEN << id << ITEM_SIZE_BEGIN_TOKEN << sz << ITEM_SIZE_END_TOKEN;

        for (int i = 0; i < sz; i++) {
            ostr << _byte_buffer[entry.index_in_buffer + i];
        }
    }

    ostr.close();

    _byte_buffer.clear();
    _str_id_to_entry_map.clear();
}