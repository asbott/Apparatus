#include <iostream>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <ostream>

#include <vector>

#include "io.h"


#include <string>

class Token {
public:
    enum class Kind {
        Number,
        Identifier,
        LeftParen,
        RightParen,
        LeftSquare,
        RightSquare,
        LeftCurly,
        RightCurly,
        LessThan,
        GreaterThan,
        Equal,
        Plus,
        Minus,
        Asterisk,
        Slash,
        Hash,
        Dot,
        Comma,
        Colon,
        Semicolon,
        SingleQuote,
        DoubleQuote,
        Comment,
        Pipe,
        End,
        Unexpected,
    };

    Token(Kind kind) noexcept : m_kind{kind} {}

    Token(Kind kind, const char* beg, std::size_t len) noexcept
        : m_kind{kind}, m_lexeme(beg, len) {}

    Token(Kind kind, const char* beg, const char* end) noexcept
        : m_kind{kind}, m_lexeme(beg, std::distance(beg, end)) {}

    Kind kind() const noexcept { return m_kind; }

    void kind(Kind kind) noexcept { m_kind = kind; }

    bool is(Kind kind) const noexcept { return m_kind == kind; }

    bool is_not(Kind kind) const noexcept { return m_kind != kind; }

    bool is_one_of(Kind k1, Kind k2) const noexcept { return is(k1) || is(k2); }

    template <typename... Ts>
    bool is_one_of(Kind k1, Kind k2, Ts... ks) const noexcept {
        return is(k1) || is_one_of(k2, ks...);
    }

    std::string_view lexeme() const noexcept { return m_lexeme; }

    void lexeme(std::string_view lexeme) noexcept {
        m_lexeme = std::move(lexeme);
    }

private:
    Kind             m_kind{};
    std::string_view m_lexeme{};
};

class Lexer {
public:
    Lexer(const char* beg) noexcept : m_beg{beg} {}

    Token next() noexcept;

private:
    Token identifier() noexcept;
    Token number() noexcept;
    Token slash_or_comment() noexcept;
    Token atom(Token::Kind) noexcept;

    char peek() const noexcept { return *m_beg; }
    char get() noexcept { return *m_beg++; }

    const char* m_beg = nullptr;
};

bool is_space(char c) noexcept {
    switch (c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return true;
    default:
        return false;
    }
}

bool is_digit(char c) noexcept {
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return true;
    default:
        return false;
    }
}

bool is_identifier_char(char c) noexcept {
    switch (c) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '_':
        return true;
    default:
        return false;
    }
}

Token Lexer::atom(Token::Kind kind) noexcept { return Token(kind, m_beg++, 1); }

Token Lexer::next() noexcept {
    while (is_space(peek())) get();

    switch (peek()) {
    case '\0':
        return Token(Token::Kind::End, m_beg, 1);
    default:
        return atom(Token::Kind::Unexpected);
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
        return identifier();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return number();
    case '(':
        return atom(Token::Kind::LeftParen);
    case ')':
        return atom(Token::Kind::RightParen);
    case '[':
        return atom(Token::Kind::LeftSquare);
    case ']':
        return atom(Token::Kind::RightSquare);
    case '{':
        return atom(Token::Kind::LeftCurly);
    case '}':
        return atom(Token::Kind::RightCurly);
    case '<':
        return atom(Token::Kind::LessThan);
    case '>':
        return atom(Token::Kind::GreaterThan);
    case '=':
        return atom(Token::Kind::Equal);
    case '+':
        return atom(Token::Kind::Plus);
    case '-':
        return atom(Token::Kind::Minus);
    case '*':
        return atom(Token::Kind::Asterisk);
    case '/':
        return slash_or_comment();
    case '#':
        return atom(Token::Kind::Hash);
    case '.':
        return atom(Token::Kind::Dot);
    case ',':
        return atom(Token::Kind::Comma);
    case ':':
        return atom(Token::Kind::Colon);
    case ';':
        return atom(Token::Kind::Semicolon);
    case '\'':
        return atom(Token::Kind::SingleQuote);
    case '"':
        return atom(Token::Kind::DoubleQuote);
    case '|':
        return atom(Token::Kind::Pipe);
    }
}

Token Lexer::identifier() noexcept {
    const char* start = m_beg;
    get();
    while (is_identifier_char(peek())) get();
    return Token(Token::Kind::Identifier, start, m_beg);
}

Token Lexer::number() noexcept {
    const char* start = m_beg;
    get();
    while (is_digit(peek())) get();
    return Token(Token::Kind::Number, start, m_beg);
}

Token Lexer::slash_or_comment() noexcept {
    const char* start = m_beg;
    get();
    if (peek() == '/') {
        get();
        start = m_beg;
        while (peek() != '\0') {
            if (get() == '\n') {
                return Token(Token::Kind::Comment, start,
                    std::distance(start, m_beg) - 1);
            }
        }
        return Token(Token::Kind::Unexpected, m_beg, 1);
    } else {
        return Token(Token::Kind::Slash, start, 1);
    }
}

#include <iomanip>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const Token::Kind& kind) {
    static const char* const names[]{
        "Number",      "Identifier",  "LeftParen",  "RightParen", "LeftSquare",
        "RightSquare", "LeftCurly",   "RightCurly", "LessThan",   "GreaterThan",
        "Equal",       "Plus",        "Minus",      "Asterisk",   "Slash",
        "Hash",        "Dot",         "Comma",      "Colon",      "Semicolon",
        "SingleQuote", "DoubleQuote", "Comment",    "Pipe",       "End",
        "Unexpected",
    };
    return os << names[static_cast<int>(kind)];
}

struct Tag {
    Tag(const std::string& str, const std::set<std::string>& args) 
        : str(str), args(args) {}
    Tag(const std::string& str) : str(str), args({}) {}
    std::string str;
    std::set<std::string> args;

    friend bool operator < (const Tag& lhs, const Tag& rhs) {
        return lhs.str < rhs.str;
    }

    bool operator ==(const std::string& rhs) {
        return str == rhs;
    }

    operator std::string& () {
        return str;
    }
};

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
        system("pause");
    }
    std::cout << "====================================================================================\n";

    //std::cin.get();

    exit(code);
}

int main(int argc, char** argv) {
    (void)argc;
	system("@echo on");
	
	std::cout << "====================================================================================\n";
	std::cout << "Running AP parser\n\n";

    output_file = argv[1];//"D:\\dev\\Apparatus\\modules\\ecs_2d_renderer\\_generated.cpp";//;
    target_directory = argv[2];//"D:\\dev\\Apparatus\\modules\\ecs_2d_renderer";//argv[2];
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

    std::set<std::string> all_types = {
        "s32", "u32", "s64", "u64", "int",
        "float", "f32", "double", "f64",
        "bool",
        "mz::ivec2", "mz::ivec3", "mz::ivec4",
        "mz::fvec2", "mz::fvec3", "mz::fvec4"
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
                if (word.find("tag") == 0) {
                    files.emplace(file_path);
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

#include <entt/meta/meta.hpp>
#include <entt/meta/resolve.hpp>

#include <misc/cpp/imgui_stdlib.h>
#include <asset_manager/asset_manager.h>

Hash_Set<uintptr_t> runtime_ids;
Hash_Map<std::string, uintptr_t> name_id_map;
Hash_Map<uintptr_t, Component_Info> component_info;

template <typename type_t>
void do_gui(const std::string& name, type_t* data) {

    std::string label = name;

    if constexpr (std::is_same<type_t, bool>()) {
        ImGui::RCheckbox(label.c_str(), data);
    } else if constexpr (std::is_integral<type_t>()) {
        ImGui::RDragInt(label.c_str(), (s32*)data, .1f);
    } else if constexpr (std::is_same<type_t, mz::ivec2>()) {
        ImGui::RDragInt2(label.c_str(), (s32*)data, .1f);
    } else if constexpr (std::is_same<type_t, mz::ivec3>()) {
        ImGui::RDragInt3(label.c_str(), (s32*)data, .1f);
    } else if constexpr (std::is_same<type_t, mz::ivec4>()) {
        ImGui::RDragInt4(label.c_str(), (s32*)data, .1f);
    } else if constexpr (std::is_same<type_t, f32>()) {
        ImGui::RDragFloat(label.c_str(), (f32*)data, 0.1f);
    } else if constexpr (std::is_same<type_t, mz::fvec2>()) {
        ImGui::RDragFvec2(label.c_str(), data, 0.1f);
    } else if constexpr (std::is_same<type_t, mz::fvec3>()) {
        ImGui::RDragFvec3(label.c_str(), data, 0.1f);
    } else if constexpr (std::is_same<type_t, mz::fvec4>()) {
        ImGui::RDragFvec4(label.c_str(), data, 0.1f);
    } else {
        ImGui::Text("%s N/A", label.c_str());
    }
}

Module* g_self = NULL;

Module* get_self() {
    return g_self;
}

module_scope {

    // Generated
	module_function(void) init(Module* self) {
    g_self = self;

)"; constexpr char lower_code[] = R"(
    
    }

    module_function(Component_Info*) get_component_info(uintptr_t runtime_id) {
		return &component_info[runtime_id];
	}

	module_function(const Hash_Set<uintptr_t>&)get_component_ids() {
		return runtime_ids;
	}

	module_function(void*) create_component(uintptr_t runtime_id, entt::registry& reg, entt::entity entity) {
		return component_info[runtime_id].create(reg, entity);
	}

    module_function(void*) get_component(uintptr_t runtime_id, entt::registry& reg, entt::entity entity) {
        return component_info[runtime_id].get(reg, entity);
    }

    module_function(void) remove_component(uintptr_t runtime_id, entt::registry& reg, entt::entity entity) {
		component_info[runtime_id].remove(reg, entity);
	}

    module_function(uintptr_t) get_component_id(const std::string& name) {
        if (name_id_map.find(name) != name_id_map.end())
            return name_id_map[name];
        else
            return 0;
    }

    module_function(void) set_imgui_context(ImGuiContext* ctx) {
        ImGui::SetCurrentContext(ctx);
    }

}

)";


    output_stream << upper_code;

    

    typedef std::set<Tag> Tag_List;

    struct Field_Item {
        std::string name;
        std::string property_type;
        Tag_List tags;
    };

    struct Struct_Item {
        Tag_List tags;
        std::string name;
        std::vector<Field_Item> members;
    };

    /*Tagged_Item current_item;
    Tagged_Item* current_parent = NULL;*/

    std::vector<Struct_Item> structs;
    std::vector<Token> previous_tokens;
    Tag_List tags;

    bool in_struct = false;

    for (auto& file_path : files) {
        std::cout << "\nDoing file " << file_path << "\n";
        auto src = read_file(file_path);
        Lexer lex(src.c_str());

        #define is_end(t) t.is(Token::Kind::End)
        for (auto token = lex.next(); !is_end(token); token = lex.next()) {
            if (token.is(Token::Kind::Semicolon) && previous_tokens[previous_tokens.size() - 1].is(Token::Kind::RightCurly)) {
                in_struct = false;
            }
            if (token.is(Token::Kind::Identifier) && token.lexeme() == "tag") {
                token = lex.next();
                if (!token.is(Token::Kind::LeftParen)) {
                    std::cout << red << "Expected '(' for tag list open\n" << white;
                    done(-1);
                }
                token = lex.next();
                while (!is_end(token) && token.is_not(Token::Kind::RightParen)) {
                    if (!token.is_one_of(Token::Kind::Comma, Token::Kind::Identifier)) {
                        std::cout << red << "Unexpected token'" << token.lexeme() << "' in tags list" << white;
                        done(-1);
                    }
                    if (token.is(Token::Kind::Identifier)) {

                        bool is_args = false;
                        auto next = lex.next();
                        std::set<std::string> args;
                        if (next.is(Token::Kind::LeftParen)) {
                            is_args = true;
                            std::string str_arg = "";
                            bool in_str = false;
                            next = lex.next();
                            while (!next.is(Token::Kind::RightParen)) {
                                std::cout << next.lexeme() << " (" << next.kind() << "), " << Token::Kind::SingleQuote << ", " << Token::Kind::DoubleQuote << "\n";
                                if (next.is(Token::Kind::DoubleQuote) && !in_str) {
                                    std::cout << next.lexeme() << " is an opening single quote\n";
                                    in_str = true;
                                    next = lex.next();
                                }
                                if (next.is(Token::Kind::DoubleQuote) && in_str) {
                                    std::cout << next.lexeme() << " is a closing single quote\n";
                                    in_str = false;
                                    std::cout << "found string arg: " << str_arg << "\n";
                                    args.emplace(str_arg);
                                    str_arg = "";
                                    next = lex.next();
                                }

                                if (next.is(Token::Kind::Identifier) || next.is(Token::Kind::Number)) {
                                    std::cout << next.lexeme() << " , " << in_str << "\n";
                                    if (in_str) {
                                        str_arg += next.lexeme();
                                    } else { 
                                        std::cout << "Found arg (single token): " << next.lexeme() << "\n";
                                        args.emplace(next.lexeme());
                                    }
                                }

                                next = lex.next();
                            }
                        }
                        tags.emplace(std::string(token.lexeme()), args);
                        token = next;

                    } else {
                        token = lex.next();
                    }
                    
                }
            }

            if (token.is(Token::Kind::Identifier) && token.lexeme() == "struct") {
                Struct_Item item;
                item.tags = tags;
                
                Token next = lex.next();
                while (next.is_not(Token::Kind::LeftCurly)) {
                    token = next;
                    next = lex.next();
                }

                item.name = token.lexeme();

                tags.clear();
                in_struct = true;
                structs.push_back(item);
            }

            if (token.is_one_of(Token::Kind::Semicolon, Token::Kind::Equal) && previous_tokens.size() >= 2) {
                auto first = previous_tokens[previous_tokens.size()-2];
                auto second = previous_tokens[previous_tokens.size()-1];
                auto third = token;

                if (first.is(Token::Kind::Identifier) 
                    && second.is(Token::Kind::Identifier) 
                    && third.is_one_of(Token::Kind::Semicolon, Token::Kind::Equal)
                    && structs.size() > 0
                    && in_struct) {

                    auto& struct_item = structs[structs.size() - 1];

                    Field_Item item;
                    item.property_type = first.lexeme();
                    item.name = second.lexeme();
                    item.tags = tags;
                    tags.clear();
                    struct_item.members.push_back(item);
                }
            }

            previous_tokens.push_back(token);
        }
    }

    for (auto& struct_item : structs) {
        for (auto& tag : struct_item.tags) {
            if (tag.str == "component") {
                output_stream << "        {\n";
                output_stream << "            uintptr_t id = (uintptr_t)typeid(" << struct_item.name << ").name();\n";
                output_stream << "            runtime_ids.emplace(id);\n";
                output_stream << "            name_id_map[\"" << struct_item.name << "\"] = id;\n";
                output_stream << "            component_info[id] = {\n";
                output_stream << "                [](entt::registry& reg, entt::entity entity) { \n";
                output_stream << "                    return &reg.emplace<" << struct_item.name << ">(entity);\n";
                output_stream << "                },\n";
                output_stream << "                [](entt::registry& reg, entt::entity entity) { \n";
                output_stream << "                    if (!reg.has<" << struct_item.name << ">(entity)) return (void*)NULL;\n";
                output_stream << "                    return (void*)&reg.get<" << struct_item.name << ">(entity);\n";
                output_stream << "                }, \n";
                output_stream << "                [](entt::registry& reg, entt::entity entity) { \n";
                output_stream << "                    reg.remove<" << struct_item.name << ">(entity);\n";
                output_stream << "                },\n";
                output_stream << "            \n";
                output_stream << "                \"" << struct_item.name << "\",\n";
                output_stream << "                id,\n";
                output_stream << "                " << (struct_item.tags.find(Tag("custom_gui")) != struct_item.tags.end() ? "true" : "false")  << ",\n";
                output_stream << "                sizeof(" << struct_item.name << "),\n";

                output_stream << "                std::vector<Property_Info> {\n";
                for (auto& member : struct_item.members) {
                    std::string this_sizeof = "sizeof(" + member.property_type + ")";
                    if (member.tags.find(Tag("property")) != member.tags.end() || struct_item.tags.find(Tag("custom_gui")) != struct_item.tags.end()) {
                        output_stream << "                    Property_Info { \n";
                        output_stream << "                        [](void* data) {\n";

                        if (struct_item.tags.find(Tag("custom_gui")) != struct_item.tags.end()) {
                            output_stream << "                            on_gui((" << struct_item.name << "*)data);\n";
                        } else if (member.tags.find(Tag("string"))  != member.tags.end()) {
                            output_stream << "                            std::string s = (char*)data;\n";
                            output_stream << "                            ImGui::RInputText(\"" << member.name << "\", &s);\n";
                            output_stream << "                            strcpy((char*)data, s.c_str());\n";
                        } else if (member.tags.find(Tag("color"))  != member.tags.end()) {
                            output_stream << "                            ImGui::RColorEdit4(\"" << member.name << "\", (f32*)data);\n";
                        } else if (member.tags.find(Tag("entity"))  != member.tags.end()) {
                            output_stream << "                            ImGui::InputEntity(\"" << member.name << "\", (entt::entity*)data);\n";
                        } else if (member.tags.find(Tag("asset"))  != member.tags.end() && (*member.tags.find(Tag("asset"))).args.size() > 0) {
                            auto& arg = *(*member.tags.find(Tag("asset"))).args.begin();
                            output_stream << "                            ImGui::InputAsset(\"" << member.name << "\", (asset_id_t*)data, \"" << arg << "\");";
                        } else { 
                            output_stream << "                            do_gui<" << member.property_type << ">(\"" << member.name << "\", (" << member.property_type << "*)data);\n";
                        }
                        
                        output_stream << "                        },\n";

                        output_stream << "                        (Property_Flag)(PROPERTY_FLAG_NONE";
                        if (member.tags.find(Tag("entity"))  != member.tags.end()) {
                            output_stream << " | PROPERTY_FLAG_ENTITY";
                        }
                        output_stream << "),\n";

                        output_stream << "                        \"" << member.name << "\",\n";
                        output_stream << "                        " << this_sizeof << ",\n";
                        output_stream << "                        ap_offsetof(" << struct_item.name << ", " << member.name << "),\n";
                        output_stream << "                    },\n";


                    }
                }
                output_stream << "                }\n";
                output_stream << "            };\n";
                output_stream << "        }\n";
            }
        }
        
    }

    output_stream << lower_code;

    output_stream << "module_scope {\n    module_function(void) deinit() {\n";

    for (auto& struct_item : structs) {
        for (auto& tag : struct_item.tags) {
            if (tag.str == "component") {
                output_stream << "        entt::resolve<" << struct_item.name << ">().reset();\n";
            }
        }
    }

    output_stream << "    }\n}\n";

	done(0);
}
