#pragma once

#include "thread_server.h"

#include "module.h"

#include <imgui.h>



struct Property_Info {
	std::function<void(void*, ImGuiContext* ctx)> on_gui;

	std::string name;
	size_t size;
	size_t offset;
};

struct Entity_Info {
	entt::entity id;
	str_t<128> name;
};

struct Component_Info {
	std::function<void*(entt::registry& reg, entt::entity entity)> create;
	std::function<void*(entt::registry& reg, entt::entity entity)> get;
	std::function<void(entt::registry& reg, entt::entity entity)> remove;

	std::string name;
	uintptr_t runtime_id;
	bool has_custom_gui;

	std::vector<Property_Info> properties;
};

struct Gui_Window {
	bool open;
	name_str_t name;
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

namespace ImGui {
	inline void DoGuiWindow(Gui_Window* wnd, const std::function<void()>& fn, ImGuiWindowFlags flags = ImGuiWindowFlags_None) {
		if (wnd->open) {
			ImGui::Begin(wnd->name, &wnd->open, flags);
			fn();
			ImGui::End();
		}
	}

	

}

struct Graphics_Context;

void AP_API quit();

AP_API Thread_Server& get_thread_server();
AP_API thread_id_t get_graphics_thread();

AP_API entt::registry& get_entity_registry();

AP_API Module* get_module(name_str_t str_id);

AP_API void register_gui_window(Gui_Window* wnd);
AP_API void unregister_gui_window(Gui_Window* wnd);

AP_API void register_gui_popup(Gui_Popup* pop);
AP_API void unregister_gui_popup(Gui_Popup* pop);

AP_API int start(int argc, char** argv);