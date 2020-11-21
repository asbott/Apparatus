#pragma once

#include "apparatus.h"

typedef u32 asset_request_id_t;
typedef u64 asset_id_t;
typedef size_t data_index_t;
typedef u64 asset_type_t;

constexpr data_index_t NULL_DATA_INDEX = (data_index_t)-1;
constexpr asset_id_t NULL_ASSET_ID = (asset_id_t)-1;




/*struct Texture_Data {
	graphics_id_t graphics_id;
	ivec2 size;
	s32 channels;
	void* data;
};*/

struct Asset {
	path_str_t path = "";
	path_str_t meta_path = "";
	name_str_t file_name = "";
	name_str_t name = "";
	str16_t extension = "";

	name_str_t type = "";

	Dynamic_Array<byte>* data_stream;
	bool in_memory = false;

	u32 usage_points = 0;

	bool in_use = false;
	data_index_t data_index;

	bool is_garbage = true; // Needs to default to true so when assets array is
						    // resized, uninitialized assets are marked as garbage

	asset_id_t id;

	void* load_param = NULL;
	index_t loader = 0;

	Icon_Type icon = ICON_TYPE_FILE;

	// Only guaranteed to be valid if asset is in memory
	size_t runtime_data_size = 0;
	
	bool is(str_ptr_t str_id) {
		return strcmp(type, str_id) == 0;
	}

	template <typename type_t>
	inline type_t* get_runtime_data() {
		if (in_memory) {
			ap_assert(this->runtime_data_size == sizeof(type_t), "Runtime data size mismatch");
			return (type_t*)&(*data_stream)[data_index];
		} else {
			return NULL;
		}
	}
};

struct Asset_Loader_Specification {
	Asset_Loader_Specification() {}
	Hash_Set<Dynamic_String> extensions;
	std::function<size_t(str_ptr_t, void*)> tell_size;
	std::function<byte*(byte*, str_ptr_t, void*)> load;
	std::function<void(byte*)> unload;
	std::function<void(void*)> on_gui; 
	std::function<void(void*)> set_default_params;
	size_t param_size;
	name_str_t name;
	bool creatable = false;
	Icon_Type icon = ICON_TYPE_FILE;
	size_t runtime_data_size = 0;
};

struct Asset_Manager_Function_Library {
	typedef Asset*(*begin_use_t)(asset_id_t aid);
	typedef void(*end_use_t)(asset_id_t aid);
	typedef bool(*validate_t)(asset_id_t* paid);
	typedef Asset*(*view_t)(asset_id_t aid);
	typedef void(*register_loader_t)(const Asset_Loader_Specification& spec);
	typedef void(*unregister_loader_t)(str_ptr_t name);

	begin_use_t         begin_use         = NULL;
	end_use_t           end_use           = NULL;
	validate_t          validate          = NULL;
	view_t              view              = NULL;
	register_loader_t   register_loader   = NULL;
	unregister_loader_t unregister_loader = NULL;
};

namespace ImGui {
	// Always call this BEFORE calling begin_use on the asset. This function
	// may invalidate the asset id (set it to null)
	inline bool InputAsset(str_ptr_t label, asset_id_t* aid, str_ptr_t type) {
		auto asset_mod = get_module("asset_manager");
		if (!asset_mod || !asset_mod->get_function_library()) return false;
		auto* asset_functions = (Asset_Manager_Function_Library*)asset_mod->get_function_library();
		Asset* asset_view = asset_functions->view(*aid);
		static char na[] = "<none>";
		if (asset_view) ImGui::RInputText(label, asset_view->file_name, sizeof(asset_view->file_name), ImGuiInputTextFlags_ReadOnly);
		else ImGui::RInputText(label, na, sizeof(na), ImGuiInputTextFlags_ReadOnly);
		if (ImGui::BeginDragDropTarget()) {
			auto* p = ImGui::AcceptDragDropPayload("asset");
			if (p) {
				auto payload = (Gui_Payload*)p->Data;
				auto new_id = (asset_id_t)(uintptr_t)payload->value;
				asset_view = asset_functions->view(new_id);
				if (asset_view && asset_view->is(type)) {
					*aid = new_id;
					return true;
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (asset_view) {
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				*aid = NULL_ASSET_ID;
				return true;
			}
		}

		return false;
	}
}