#include <iostream>
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

bool read_as_string(const char* path, char* out_buffer, size_t buffer_size) {
    FILE* file_handle = fopen(path, "r");

    if (!file_handle) return false;

    fgets(out_buffer, (int)buffer_size, file_handle);

    fclose(file_handle);

    return true;
}

std::string read_file(const std::string& path) {
    char* buf = new char[1024 * 1000 * 2];
    read_as_string(path.c_str(), buf, 1024 * 1000 * 2);

    std::string result = buf;
    delete buf;
    return result;
}

inline std::ostream& blue(std::ostream &s)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE
        |FOREGROUND_GREEN|FOREGROUND_INTENSITY);
    return s;
}

inline std::ostream& red(std::ostream &s)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, 
        FOREGROUND_RED|FOREGROUND_INTENSITY);
    return s;
}

inline std::ostream& green(std::ostream &s)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, 
        FOREGROUND_GREEN|FOREGROUND_INTENSITY);
    return s;
}

inline std::ostream& yellow(std::ostream &s)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, 
        FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
    return s;
}

inline std::ostream& white(std::ostream &s)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout, 
        FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
    return s;
}

struct color {
    color(WORD attribute):m_color(attribute){};
    WORD m_color;
};

template <class _Elem, class _Traits>
std::basic_ostream<_Elem,_Traits>& 
operator<<(std::basic_ostream<_Elem,_Traits>& i, color& c)
{
    HANDLE hStdout=GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout,c.m_color);
    return i;
}

namespace fs = std::filesystem;

std::string output_file;
std::string target_directory;
std::ofstream output_stream;

void done(int code) {
    output_stream.close();
    if (code == 0) {
        std::cout << green << "\nFinished without errors\n" << white;
    } else {
        std::cout << red << "\nFinished with errors\n" << white;
    }
    std::cout << "====================================================================================\n";
    system("pause");

    exit(code);
}

int main(int argc, char** argv) {
	system("@echo on");
	
	std::cout << "====================================================================================\n";
	std::cout << "Running AP parser\n\n";

    output_file = argv[1];
    target_directory = argv[2];
    if (target_directory[target_directory.length() - 1] == '"') target_directory.pop_back();
	
    std::cout << output_file << "\n";
    if (!fs::exists(output_file)) {
        std::cout << red << "Output file not found. Make sure _generated.cpp is in this project.\n" << white;
        done(-1);
    }

    if (!fs::exists(target_directory)) {
        std::cout << red << "Target directory '" << target_directory << "' not found\n" << white;
        done(-1);
    }

    
    output_stream.open(output_file);

    if (!output_stream.good()) {
        std::cout << red << "Could not open output file, it may be used by another application.\n" << white;
        done(-1);
    }

    output_stream << "#include \"apparatus.h\"\n";

    std::set<std::string> all_tags = {
        "#component", "#serializable"
    };

    std::set<std::string> files;

    for (const auto& entry : fs::recursive_directory_iterator(target_directory)) {
        std::string file_path = entry.path().generic_string();
        std::string ext = entry.path().extension().generic_string();
        if (ext == ".h" || ext == ".hpp") {
            std::ifstream file_stream;
            file_stream.open(file_path);

            std::string word;
            while (file_stream >> word) {
                std::cout << word << "\n";
                if (word[0] == '#' && all_tags.find(word) != all_tags.end()) {
                    files.emplace(file_path);
                    std::cout << file_path << "\n";
                    output_stream << "#include \"" << file_path << "\"\n";
                    break;
                }
            }

            file_stream.close();
        }
    }

    constexpr char upper_code[] = R"(
#include <vector>
#include <functional>

Hash_Set<uintptr_t> runtime_ids;
Hash_Map<std::string, uintptr_t> name_id_map;
Hash_Map<uintptr_t, Component_Info> component_info;

extern "C" {

    // Generated
	__declspec(dllexport) void __cdecl init() {

)"; constexpr char lower_code[] = R"(
    
    }

    __declspec(dllexport) Component_Info* __cdecl get_component_info(uintptr_t runtime_id) {
		return &component_info[runtime_id];
	}

	__declspec(dllexport) const Hash_Set<uintptr_t>& __cdecl get_component_ids() {
		return runtime_ids;
	}

	__declspec(dllexport) void* __cdecl create_component(uintptr_t runtime_id, entt::registry& reg, entt::entity entity) {
		return component_info[runtime_id].create(reg, entity);
	}

    __declspec(dllexport) void __cdecl remove_component(uintptr_t runtime_id, entt::registry& reg, entt::entity entity) {
		component_info[runtime_id].remove(reg, entity);
	}

    __declspec(dllexport) uintptr_t __cdecl get_component_id(const std::string& name) {
        return name_id_map[name];
    }

}

)";


    output_stream << upper_code;
    
    enum Item_Type {
        item_type_struct, item_type_global_function, item_type_global_variable, 
        item_type_member_function, item_type_member_variable
    };

    struct Tagged_Item {
        std::set<std::string> tags;
        Item_Type type;
        std::string name;
    };

    Tagged_Item current_item;

    std::vector<Tagged_Item> items;

    for (auto& file_path : files) {
        std::ifstream input_stream;
        input_stream.open(file_path);
        
        std::string word;
        while (input_stream >> word) {
            auto it = all_tags.find(word);
            if (it != all_tags.end()) {
                std::string tag = *it;
                current_item.tags.emplace(tag);
            }

            if (word == "struct") {
                current_item.type = item_type_struct;
                input_stream >> current_item.name;

                items.push_back(current_item);
                current_item = Tagged_Item();
            }
        }

        input_stream.close();
    }

    for (auto& item : items) {
        output_stream << "        {\n";
        switch (item.type) {
        case item_type_struct:
            for (auto& tag : item.tags) {
                if (tag == "#component") {
                    output_stream << "            uintptr_t id = (uintptr_t)typeid(" << item.name << ").name();\n";
                    output_stream << "            runtime_ids.emplace(id);\n";
                    output_stream << "            name_id_map[\"" << item.name << "\"] = id;\n";
                    output_stream << "            component_info[id] = {\n";
                    output_stream << "                [](entt::registry& reg, entt::entity entity) { \n";
                    output_stream << "                    return &reg.emplace<" << item.name << ">(entity);\n";
                    output_stream << "                },\n";
                    output_stream << "                [](entt::registry& reg, entt::entity entity) { \n";
                    output_stream << "                    return &reg.get<" << item.name << ">(entity);\n";
                    output_stream << "                }, \n";
                    output_stream << "                [](entt::registry& reg, entt::entity entity) { \n";
                    output_stream << "                    reg.remove<" << item.name << ">(entity);\n";
                    output_stream << "                },\n";
                    output_stream << "            \n";
                    output_stream << "                \"" << item.name << "\",\n";
                    output_stream << "                id\n";
                    output_stream << "            };\n";
                }
            }
            break;
        default:
            break;
        }
        output_stream << "        }\n";
    }

    output_stream << lower_code;

	done(0);
}

/*
start /wait parser.exe
if %ERRORLEVEL% EQU 0 (
echo success
) else (
echo fail
)
*/

