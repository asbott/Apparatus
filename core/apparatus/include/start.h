#pragma once

#include "thread_server.h"

#include "module.h"

#include "archive.h"

#include "graphics/graphics_api.h"

#include <imgui.h>

typedef str_t<256> entity_name_t;
typedef str_t<128> comp_name_t;

	constexpr u32 MAX_COMPONENTS_PER_ENTITY = 256;

struct Property_Info {
	std::function<void(void*)> on_gui;

	std::string name;
	size_t size;
	size_t offset;
};

struct Entity_Info {
	entt::entity id;
	entity_name_t name;
};

struct Component_Info {
	std::function<void*(entt::registry& reg, entt::entity entity)> create;
	std::function<void*(entt::registry& reg, entt::entity entity)> get;
	std::function<void(entt::registry& reg, entt::entity entity)> remove;

	std::string name;
	uintptr_t runtime_id;
	bool has_custom_gui;
	size_t size;

	std::vector<Property_Info> properties;
};

struct Game_Input {
	Input_State state;

	inline bool is_mouse_down(input_code_t code) {
		return state.mouse_down[code];
	}
	inline bool is_mouse_pressed(input_code_t code) {
		return state.mouse_press[code];
	}
	inline bool is_key_down(input_code_t code) {
		return state.keys_down[code];
	}
	inline bool is_key_pressed(input_code_t code) {
		return state.keys_press[code];
	}
	inline mz::fvec2 get_mouse_pos() {
		return state.mouse_pos;
	}

	mz::fvec2 mouse_world_pos;
};

struct Gui_Window {
	Gui_Window(bool open, str_ptr_t _name, str_ptr_t _tree_path = "Windows") : open(open) {
		strcpy(name, _name);
		strcpy(tree_path, _tree_path);
	}
	bool open;
	bool focused = false;
	name_str_t name;
	path_str_t tree_path = "Windows";
};

struct Gui_Popup {
	name_str_t str_id;
	std::function<void()> fn;
	std::function<void()> done_fn = NULL;
	bool should_open = false;
	bool is_modal;
	path_str_t return_value = "";
};

struct Gui_Payload {
	void* value;
	void* home;
};

enum Icon_Type : u8 {
	ICON_TYPE_STOP, ICON_TYPE_PLAY, ICON_TYPE_OPTIONS, ICON_TYPE_TEXTURE,
	ICON_TYPE_FOLDER, ICON_TYPE_FILE,

	ICON_TYPE_COUNT
};

#include "imgui_extension.h"

namespace ImGui {
	inline void DoGuiWindow(Gui_Window* wnd, const std::function<void()>& fn, ImGuiWindowFlags flags = ImGuiWindowFlags_None) {
		if (wnd->open) {
			ImGui::Begin(wnd->name, &wnd->open, flags);
			wnd->focused = ImGui::IsWindowFocused();
			fn();
			ImGui::End();
		}
	}

	AP_API void Icon(Icon_Type icon, mz::ivec2 size);
	AP_API bool IconButton(Icon_Type icon, mz::ivec2 size, const mz::color& bgr_color = 0, bool border = false);
}

enum class File_Browser_Mode {
	directory, file, all
};

struct Graphics_Context;

void AP_API quit();

AP_API Graphics_Context* get_graphics();

// Get input polling for the game window
AP_API Game_Input* game_input();

AP_API Thread_Server& get_thread_server();
AP_API thread_id_t get_graphics_thread();

AP_API entt::registry& get_entity_registry();

AP_API void register_module(str_ptr_t mod_name);
AP_API Module* get_module(str_ptr_t str_id);

AP_API void register_gui_window(Gui_Window* wnd);
AP_API void unregister_gui_window(Gui_Window* wnd);

AP_API void register_gui_popup(Gui_Popup* pop);
AP_API void unregister_gui_popup(Gui_Popup* pop);

AP_API void select_entity(entt::entity entity);
AP_API void deselect_entity(entt::entity entity);
AP_API void deselect_all_entities();
AP_API bool is_entity_selected(entt::entity entity);
AP_API bool is_any_entity_selected();
AP_API const Hash_Set<entt::entity>& get_selected_entities();

AP_API str_ptr_t get_user_directory();

AP_API void open_file_browser(File_Browser_Mode mode, std::function<void(str_ptr_t)> result_callback = 0);

AP_API bool is_playing();

// Defer code to run after current module function has been
// invoked in other modules
AP_API void defer(std::function<void()> fn);

AP_API int start(int argc, char** argv);