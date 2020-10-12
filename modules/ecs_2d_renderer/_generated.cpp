#include "apparatus.h"
#include "D:/dev/Apparatus/modules/ecs_2d_renderer/ecs_2d_renderer.h"

#include <vector>
#include <functional>

#include <misc/cpp/imgui_stdlib.h>

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
        ImGui::RDragFloat2(label.c_str(), (f32*)data, 0.1f);
    } else if constexpr (std::is_same<type_t, mz::fvec3>()) {
        ImGui::RDragFloat3(label.c_str(), (f32*)data, 0.1f);
    } else if constexpr (std::is_same<type_t, mz::fvec4>()) {
        ImGui::RDragFloat4(label.c_str(), (f32*)data, 0.1f);
    } else {
        ImGui::Text("%s N/A", label.c_str());
    }
}

extern "C" {

    // Generated
	__declspec(dllexport) void __cdecl init() {

        {
            uintptr_t id = (uintptr_t)typeid(Transform2D).name();
            runtime_ids.emplace(id);
            name_id_map["Transform2D"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<Transform2D>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<Transform2D>(entity)) return (void*)NULL;
                    return (void*)&reg.get<Transform2D>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<Transform2D>(entity);
                },
            
                "Transform2D",
                id,
                false,
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            do_gui<fvec2>("position", (fvec2*)data);
                        },
                        "position",
                        sizeof(fvec2),
                        0,
                    },
                    Property_Info { 
                        [](void* data) {
                            do_gui<f32>("rotation", (f32*)data);
                        },
                        "rotation",
                        sizeof(f32),
                        sizeof(fvec2),
                    },
                    Property_Info { 
                        [](void* data) {
                            do_gui<fvec2>("scale", (fvec2*)data);
                        },
                        "scale",
                        sizeof(fvec2),
                        sizeof(fvec2)+sizeof(f32),
                    },
                }
            };
        }
        {
            uintptr_t id = (uintptr_t)typeid(Sprite2D).name();
            runtime_ids.emplace(id);
            name_id_map["Sprite2D"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<Sprite2D>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<Sprite2D>(entity)) return (void*)NULL;
                    return (void*)&reg.get<Sprite2D>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<Sprite2D>(entity);
                },
            
                "Sprite2D",
                id,
                false,
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            (void)data;
                            Asset_Request_View view_request;
                            view_request.asset_id = *(asset_id_t*)data;
                            Asset* asset_view = get_module("asset_manager")->request<Asset*>(view_request);
                            char na[] = "<none>";
                            if (asset_view) ImGui::RInputText("texture", asset_view->file_name, strlen(asset_view->file_name));
                            else ImGui::RInputText("texture", na, strlen(na));
                            if (ImGui::BeginDragDropTarget()) {
                                auto* p = ImGui::AcceptDragDropPayload("asset");
                                if (p) {
                                    auto payload = (Gui_Payload*)p->Data;
                                    auto new_id = (asset_id_t)(uintptr_t)payload->value;
                                    view_request.asset_id = new_id;
                                    asset_view = get_module("asset_manager")->request<Asset*>(view_request);
                                    if (asset_view && asset_view->asset_type == ASSET_TYPE_TEXTURE) memcpy(data, &new_id, sizeof(asset_id_t));
                                }
                            ImGui::EndDragDropTarget();
                            }
                        },
                        "texture",
                        sizeof(asset_id_t),
                        0,
                    },
                    Property_Info { 
                        [](void* data) {
                            ImGui::RColorEdit4("tint", (f32*)data);
                        },
                        "tint",
                        sizeof(color),
                        sizeof(asset_id_t),
                    },
                    Property_Info { 
                        [](void* data) {
                            do_gui<fvec2>("origin", (fvec2*)data);
                        },
                        "origin",
                        sizeof(fvec2),
                        sizeof(asset_id_t)+sizeof(color),
                    },
                }
            };
        }
        {
            uintptr_t id = (uintptr_t)typeid(View2D).name();
            runtime_ids.emplace(id);
            name_id_map["View2D"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<View2D>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<View2D>(entity)) return (void*)NULL;
                    return (void*)&reg.get<View2D>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<View2D>(entity);
                },
            
                "View2D",
                id,
                false,
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            ImGui::RColorEdit4("clear_color", (f32*)data);
                        },
                        "clear_color",
                        sizeof(color16),
                        0,
                    },
                }
            };
        }

    
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

    __declspec(dllexport) void* __cdecl get_component(uintptr_t runtime_id, entt::registry& reg, entt::entity entity) {
        return component_info[runtime_id].get(reg, entity);
    }

    __declspec(dllexport) void __cdecl remove_component(uintptr_t runtime_id, entt::registry& reg, entt::entity entity) {
		component_info[runtime_id].remove(reg, entity);
	}

    __declspec(dllexport) uintptr_t __cdecl get_component_id(const std::string& name) {
        if (name_id_map.find(name) != name_id_map.end())
            return name_id_map[name];
        else
            return 0;
    }

    __declspec(dllexport) void __cdecl set_imgui_context(ImGuiContext* ctx) {
        ImGui::SetCurrentContext(ctx);
    }

}

