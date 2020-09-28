#include "apparatus.h"

#include "asset_manager.h"

#include "pfd/portable-file-dialogs.h"

struct Asset_Directory {
	Asset_Directory(Asset_Directory* parent, str_ptr_t path) 
		: parent_directory(parent) {
		Path::name_with_extension(path, name);
		strcpy(real_path, path);
	}

	Asset_Directory* parent_directory;

	path_str_t name;
	path_str_t real_path;

	Hash_Set<asset_id_t> assets;

	Dynamic_Array<Asset_Directory*> sub_directories;

	void traverse(const std::function<void(Asset_Directory&)> fn) {
		for (auto& sub : sub_directories) {
			sub->traverse(fn);
		}
		fn(*this);
	}
};

Gui_Window g_asset_manager = { false, "Asset Manager" };
Gui_Window g_asset_inspector = { false, "Asset Inspector" };

Gui_Popup g_file_browser_popup;
Gui_Popup g_manager_menu_popup;

path_str_t g_current_dir;
path_str_t g_essential_dir;
path_str_t g_assets_dir;

Dynamic_Array<Asset> assets;
Dynamic_Array<byte> runtime_data;

// Sorted by usage frequency
Dynamic_Array<asset_id_t> order_to_unload_assets;

u32 number_of_assets_in_use = 0;

Graphics_Context* g_graphics;

graphics_id_t g_file_icon;
graphics_id_t g_folder_icon;

Asset_Directory* g_root_directory;
Asset_Directory* g_selected_dir = NULL;

data_index_t load_texture(Asset* asset) {
	Texture_Data tex;
	
	tex.graphics_id = g_graphics->make_texture(G_BUFFER_USAGE_STATIC_WRITE);
	auto* img = load_image_from_file(asset->path, &tex.size.x, &tex.size.y, &tex.channels, 4);

	if (!img) {
		log_error("Failed loading texture from: \n{}\nReason: ", get_failure_reason());
		return NULL_DATA_INDEX;
	}

	g_graphics->set_texture_filtering(tex.graphics_id, G_MIN_FILTER_LINEAR, G_MAG_FILTER_NEAREST);
	g_graphics->set_texture_wrapping(tex.graphics_id, G_WRAP_CLAMP_TO_BORDER);

	g_graphics->set_texture_data(tex.graphics_id, img, tex.size, G_TEXTURE_FORMAT_RGBA);

	static std::thread::id this_thread = std::this_thread::get_id();
	ap_assert(this_thread == std::this_thread::get_id(), "I sayeth, Nay Thee! ({}, {})", this_thread, std::this_thread::get_id());

	size_t img_size = tex.size.width * tex.size.height * tex.channels;

	data_index_t data_index = runtime_data.size();
	runtime_data.resize(runtime_data.size() + sizeof(Texture_Data) + img_size);
	tex.data = &runtime_data[data_index + sizeof(Texture_Data)];

	memcpy(&runtime_data[data_index], &tex, sizeof(Texture_Data));
	memcpy(&runtime_data[data_index + sizeof(Texture_Data)], img, img_size);


	return data_index;
}

bool unload_texture(Asset* asset) {
	asset->is_garbage = true;

	auto& tex = *(Texture_Data*)asset->get_runtime_data();
	
	g_graphics->destroy_texture(tex.graphics_id);
	free_image((byte*)tex.data);

	return true;
}

asset_id_t register_asset(str_ptr_t path, asset_type_t asset_type) {
	
	Asset a;
	strcpy(a.path, path);
	Path::extension_of(path, a.extension);
	Path::name_with_extension(path, a.file_name);
	Path::name_without_extension(path, a.name);

	a.asset_type = asset_type;
	a.data_stream = &runtime_data;

	for (int i = 0; i < assets.size(); i++) {
		if (assets[i].is_garbage) {
			assets[i] = std::move(a);
			return i;
		}
	}

	assets.push_back(std::move(a));

	log_info("Registered asset:\nPath: {}\nName: {}, {}\nExt:  {}",
			  a.path, a.name, a.file_name, a.extension);

	asset_id_t id = (asset_id_t)assets.size() - 1;

	return id;
}

data_index_t load_to_memory(asset_id_t id) {
	ap_assert(id < assets.size());
	Asset* asset = &assets[id];
	
	data_index_t idx = load_texture(asset);
	
	if (idx == NULL_DATA_INDEX) {
		return NULL_DATA_INDEX;
	}

	asset->in_memory = true;

	return idx;
}

bool unload_from_memory(asset_id_t id) {
	ap_assert(id < assets.size());
	Asset* asset = &assets[id];
	ap_assert(!asset->in_use, "Cannot unload an asset that's in use");

	if (!unload_texture(asset)) return false;

	asset->in_memory = false;

	return true;
}

void delete_asset(asset_id_t id) {
	ap_assert(number_of_assets_in_use == 0, "Cannot delete an asset when there are assets in use.");
	ap_assert(id < assets.size());
	Asset& asset = assets[id];

	if (asset.in_memory) {
		unload_from_memory(id);
	}
}

Asset* begin_use(asset_id_t id) {
	ap_assert(id < assets.size());
	number_of_assets_in_use++;

	Asset* asset = &assets[id];
	asset->in_use = true;
	asset->usage_points++;

	if (!asset->in_memory) {
		asset->data_index = load_to_memory(id);
	}

	return asset;
}

void end_use(asset_id_t id) {
	ap_assert(id < assets.size());
	Asset* asset = &assets[id];
	ap_assert(asset->in_use, "Cannot stop using an asset that's not in use");
	asset->in_use = false;

	number_of_assets_in_use--;
}

void do_asset_manager_menu_gui(Asset_Directory* dir) {

	if (ImGui::BeginMenu("Create")) {

		static name_str_t create_name = "";

		ImGui::InputText("Name", create_name, sizeof(create_name));

		if (ImGui::MenuItem("Directory")) {
			path_str_t new_path = "";
			sprintf(new_path, "%s/%s", dir->real_path, create_name);
			if (Path::create_directory(new_path)) {
				Asset_Directory* new_dir = new Asset_Directory(dir, new_path);
				dir->sub_directories.push_back(new_dir);
			} else {
				log_error("Failed creating directory '{}'", new_path);
			}
		}

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Add")) {
		g_file_browser_popup.should_open = true;
		g_file_browser_popup.done_fn = [dir]() {
			str_ptr_t result_path = g_file_browser_popup.return_value;
			if (Path::exists(result_path)) {
				str16_t ext = "";
				Path::extension_of(result_path, ext);

				path_str_t new_path = "";
				path_str_t file_name = "";
				Path::name_with_extension(result_path, file_name);
				sprintf(new_path, "%s/%s", g_assets_dir, file_name);
				Path::copy(result_path, new_path);

				if (strcmp(ext, "png") == 0) {
					auto id = register_asset(new_path, ASSET_TYPE_TEXTURE);
					if (id != NULL_ASSET_ID) {
						dir->assets.emplace(id);
					}
				} else {
					log_error("Cannot load files with extension '{}'", ext);
				}
			}
		};
	}
}

extern "C" {
	_export void __cdecl on_load(Graphics_Context* graphics) {
		log_info("on_load called in test_module!");
		(void)graphics;

		sprintf(g_essential_dir, "%s/../../essential", get_executable_directory());
		sprintf(g_assets_dir, "%s/../../assets", get_executable_directory());

		g_root_directory = new Asset_Directory(NULL, g_assets_dir);
		g_selected_dir = g_root_directory;

		path_str_t folder_icon_path = "";
		path_str_t file_icon_path = "";

		sprintf(folder_icon_path, "%s/vendor/folder.png", g_essential_dir);
		sprintf(file_icon_path, "%s/file.png", g_essential_dir);

		mz::ivec3 folder_size;
		auto folder_img = load_image_from_file(folder_icon_path, &folder_size.x, &folder_size.y, &folder_size.z, 4);
		ap_assert(folder_img != NULL, "Failed loading folder icon: {}", get_failure_reason());

		mz::ivec3 file_size;
		auto file_img = load_image_from_file(file_icon_path, &file_size.x, &file_size.y, &file_size.z, 4);
		ap_assert(file_img != NULL, "Failed loading file icon: {}", get_failure_reason());
		
		g_folder_icon = graphics->make_texture(G_BUFFER_USAGE_STATIC_WRITE);
		g_file_icon = graphics->make_texture(G_BUFFER_USAGE_STATIC_WRITE);

		graphics->set_texture_filtering(g_folder_icon, G_MIN_FILTER_LINEAR, G_MAG_FILTER_NEAREST);
		graphics->set_texture_filtering(g_file_icon, G_MIN_FILTER_LINEAR, G_MAG_FILTER_NEAREST);

		graphics->set_texture_wrapping(g_folder_icon, G_WRAP_CLAMP_TO_BORDER);
		graphics->set_texture_wrapping(g_file_icon, G_WRAP_CLAMP_TO_BORDER);

		graphics->set_texture_data(g_folder_icon, folder_img, folder_size, G_TEXTURE_FORMAT_RGBA);
		graphics->set_texture_data(g_file_icon, file_img, file_size, G_TEXTURE_FORMAT_RGBA);

		graphics->set_clear_color(mz::color(.35f, .1f, .65f, 1.f));

		register_gui_window(&g_asset_manager);
		register_gui_window(&g_asset_inspector);

		strcpy(g_current_dir, get_executable_directory());

		strcpy(g_file_browser_popup.str_id, "File Browser");
		g_file_browser_popup.is_modal = true;
		strcpy(g_file_browser_popup.return_value, get_executable_directory());
		g_file_browser_popup.fn = [graphics]() {

			#ifdef _OS_WINDOWS
			str_t<3> current_disk = "  ";
			current_disk[0] = g_current_dir[0];
			current_disk[1] = g_current_dir[1];
			if (ImGui::BeginCombo("Disk", current_disk)) {

				for (char c = 'A'; c <= 'Z'; c++) {
					str_t<3> disk = "  ";
					disk[0] = c;
					disk[1] = ':';
					str_t<4> disk_path;
					sprintf(disk_path, "%s/", disk);
					if (!Path::exists(disk_path)) continue;
					bool is_current = strcmp(disk, current_disk) == 0;
					if (ImGui::MenuItem(disk, NULL, is_current)) {
						if (!is_current) {
							sprintf(g_current_dir, "%s/", disk);
						}
					}
				}

				ImGui::EndCombo();
			}
			#endif

			ImGui::Text("Current Directory: %s", g_current_dir);

			path_str_t prev_dir = "";
			Path::directory_of(g_current_dir, prev_dir);
			
			static Dynamic_Array<Dynamic_String> dirs_ordered;
			while (Path::exists(prev_dir)) {
				dirs_ordered.push_back(prev_dir);
				str16_t root = "";
				Path::root_name(g_current_dir, root);
				if (strcmp(root, prev_dir) == 0) break;
				Path::directory_of(prev_dir, prev_dir);
			}

			for (int i = (int)dirs_ordered.size() - 1; i >= 0; i--) {
				path_str_t name;
				Path::name_with_extension(dirs_ordered[i].c_str(), name);
				if (ImGui::Button(name)) {
					strcpy(g_current_dir, dirs_ordered[i].c_str());
				}
				ImGui::SameLine();
			}

			dirs_ordered.clear();

			if (ImGui::Button("<")) {
				Path::directory_of(g_current_dir, g_current_dir);
			}

			#ifdef _WIN32

			for (char c = 'A'; c <= 'Z'; c++) {
				if (strlen(g_current_dir) == 2 && g_current_dir[0] == c && g_current_dir[1] == ':') {
					strcat(g_current_dir, "/");
				}
			}

			#endif

			ImGui::BeginChildFrame(1, { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y * 0.65f });

			Path::iterate_directories(g_current_dir, [graphics](str_ptr_t path) {
				if (!Path::can_open(path)){
					path_str_t dir_name = "";
					path_str_t ext = "";
					Path::extension_of(path, ext);
					if (ext[0] == '\0') Path::name_without_extension(path, dir_name);
					else			    Path::name_with_extension(path, dir_name);

					ImGui::Image(graphics->get_native_texture_handle(g_folder_icon), { 16, 16 });
					ImGui::SameLine();
					if (ImGui::Selectable(dir_name)) {
						strcpy(g_current_dir, path);
					}
				}
			});

			Path::iterate_directories(g_current_dir, [graphics](str_ptr_t path) {
				if (Path::can_open(path)) {
					path_str_t file_name = "";
					path_str_t ext = "";
					Path::extension_of(path, ext);
					if (ext[0] == '\0') Path::name_without_extension(path, file_name);
					else			    Path::name_with_extension(path, file_name);

					ImGui::Image(graphics->get_native_texture_handle(g_file_icon), { 16, 16 });
					ImGui::SameLine();
					if (ImGui::Selectable(file_name, strcmp(path, g_file_browser_popup.return_value) == 0)) {
						strcpy(g_file_browser_popup.return_value, path);
					}
				}
			});

			ImGui::EndChildFrame();

			ImGui::Text("Selected file: %s", g_file_browser_popup.return_value);

			if (ImGui::Button("Ok")) {
				if (g_file_browser_popup.done_fn) {
					g_file_browser_popup.done_fn();
					g_file_browser_popup.done_fn = NULL;
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
				g_file_browser_popup.done_fn = NULL;
			}
		};

		register_gui_popup(&g_file_browser_popup);

		g_manager_menu_popup.is_modal = false;
		strcpy(g_manager_menu_popup.str_id, "Asset Manager Menu");
		g_manager_menu_popup.fn = []() {
			do_asset_manager_menu_gui(g_selected_dir);
		};

		register_gui_popup(&g_manager_menu_popup);

		g_graphics = graphics;
	}

	_export void __cdecl on_unload(Graphics_Context* graphics) {
		(void)graphics;

		unregister_gui_window(&g_asset_manager);
		unregister_gui_window(&g_asset_inspector);
	}

	_export void __cdecl on_update(float delta) {
		(void)delta;


	}

	_export void __cdecl on_render(Graphics_Context* graphics) {
		auto windows = graphics->get_windows_context();
		auto wnd = windows->main_window_handle;

		if (windows->should_close(wnd)) {
			quit();
		}
	}

	void do_dir_gui(Asset_Directory& dir, int depth = 0) {

		ImGuiTreeNodeFlags flags = 0;
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		if (strcmp(dir.real_path, g_selected_dir->real_path) == 0) 
			flags |= ImGuiTreeNodeFlags_Selected;
		path_str_t label = "";
		sprintf(label, "%s##%i", dir.name, depth);
		bool node_open = ImGui::TreeNodeEx(label, flags);
		if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) {
			g_selected_dir = &dir;
		}

		if (ImGui::BeginDragDropTarget()) {
			auto* p = ImGui::AcceptDragDropPayload("asset");
			if (p) {
				auto payload = (Gui_Payload*)p->Data;
				auto asset_id = (asset_id_t)(uintptr_t)payload->value;
				auto* prev_dir = (Asset_Directory*)payload->home;

				prev_dir->assets.erase(asset_id);
				dir.assets.emplace(asset_id);
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked(1)) {
			g_manager_menu_popup.should_open = true;
		}

		if (node_open) {	
			for (auto sub : dir.sub_directories) {
				do_dir_gui(*sub, ++depth);
			}
			for (auto asset_id : dir.assets) {
			auto& asset = assets[asset_id];

			if (ImGui::Selectable(asset.file_name)) {

			}
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
				Gui_Payload payload;
				payload.home = &dir;
				payload.value = (void*)(uintptr_t)asset_id;
				log_trace("Payload value set to {}", (uintptr_t)payload.value);
				ImGui::SetDragDropPayload("asset", &payload, sizeof(Gui_Payload));
				ImGui::Text(asset.file_name);
				ImGui::EndDragDropSource();
			}
		}
			ImGui::TreePop();
		}
		depth++;
	}

	_export void __cdecl on_gui(Graphics_Context* graphics, ImGuiContext* imgui_ctx) {
		(void)graphics;(void)imgui_ctx;

		ImGui::SetCurrentContext(imgui_ctx);

		ImGui::DoGuiWindow(&g_asset_manager, [&]() {

			ImGui::BeginMenuBar();

			do_asset_manager_menu_gui(g_selected_dir ? g_selected_dir : g_root_directory);

			ImGui::EndMenuBar();

			do_dir_gui(*g_root_directory);

		}, ImGuiWindowFlags_MenuBar);

		ImGui::DoGuiWindow(&g_asset_inspector, [&]() {

		});
	}

	_export void* __cdecl _request(void* ud) {
		Asset_Request* req = (Asset_Request*)ud;
		if (req->request_id == ASSET_REQUEST_REGISTER_ASSET) {
			auto* request = (Asset_Request_Register_Asset*)req;
			
			if (request->asset_type == ASSET_TYPE_TEXTURE) {
				return (void*)(uintptr_t)register_asset(request->path, request->asset_type);
			} else {
				log_error("Invalid asset type in register asset request");
				return NULL;
			}
		} else if (req->request_id == ASSET_REQUEST_BEGIN_USE_ASSET) {
			auto* request = (Asset_Request_Begin_Use_asset*)req;

			return begin_use(request->asset_id);
		} else if (req->request_id == ASSET_REQUEST_END_USE_ASSET) {
			auto* request = (Asset_Request_End_Use_asset*)req;

			end_use(request->asset_id);
			return NULL;
		} else if (req->request_id == ASSET_REQUEST_CHECK_IF_VALID) {
			auto* request = (Asset_Request_Check_If_Valid*)req;

			return (void*)(uintptr_t)(request->asset_id < assets.size() && !assets[request->asset_id].is_garbage);
		} else if (req->request_id == ASSET_REQUEST_VIEW) {
			auto* request = (Asset_Request_View*)req;

			if (request->asset_id < assets.size() && !assets[request->asset_id].is_garbage)
				return &assets[request->asset_id];
			else
				return NULL;
		}

		log_error("Invalid asset request");
		return NULL;
	}
}