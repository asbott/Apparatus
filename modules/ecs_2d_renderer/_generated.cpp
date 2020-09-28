#include "apparatus.h"
#include "D:/dev/Apparatus/modules/ecs_2d_renderer/ecs_2d_renderer.h"

#include <vector>
#include <functional>

#include <misc/cpp/imgui_stdlib.h>

Hash_Set<uintptr_t> runtime_ids;
Hash_Map<std::string, uintptr_t> name_id_map;
Hash_Map<uintptr_t, Component_Info> component_info;

template <typename type_t>
void do_gui(const std::string& name, type_t* data, ImGuiContext* ctx) {
    ImGui::SetCurrentContext(ctx);

    std::string label = name + "##" + std::to_string((uintptr_t)data);

    if constexpr (std::is_same<type_t, bool>()) {
        ImGui::Checkbox(label.c_str(), data);
    } else if constexpr (std::is_integral<type_t>()) {
        ImGui::InputInt(label.c_str(), (s32*)data);
    } else if constexpr (std::is_same<type_t, mz::ivec2>()) {
        ImGui::InputInt2(label.c_str(), (s32*)data);
    } else if constexpr (std::is_same<type_t, mz::ivec3>()) {
        ImGui::InputInt3(label.c_str(), (s32*)data);
    } else if constexpr (std::is_same<type_t, mz::ivec4>()) {
        ImGui::InputInt4(label.c_str(), (s32*)data);
    } else if constexpr (std::is_same<type_t, f32>()) {
        ImGui::InputFloat(label.c_str(), data, 0.1f, 0.2f, 5);
    } else if constexpr (std::is_same<type_t, mz::fvec2>()) {
        ImGui::InputFloat2(label.c_str(), (f32*)data, 5);
    } else if constexpr (std::is_same<type_t, mz::fvec3>()) {
        ImGui::InputFloat3(label.c_str(), (f32*)data, 5);
    } else if constexpr (std::is_same<type_t, mz::fvec4>()) {
        ImGui::InputFloat4(label.c_str(), (f32*)data, 5);
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
                true,
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data, ImGuiContext* ctx) {
                            ImGui::SetCurrentContext(ctx);
                            on_gui((Transform2D*)data, ctx);
                        },
                        "matrix",
                        sizeof(fmat4),
                        0,
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
                        [](void* data, ImGuiContext* ctx) {
                            (void)data;
                            ImGui::SetCurrentContext(ctx);
                            Asset_Request_View view_request;
                            view_request.asset_id = *(asset_id_t*)data;
                            Asset* asset_view = get_module("asset_manager")->request<Asset*>(&view_request);
                            char na[] = "<none>";
                            if (asset_view) ImGui::InputText("texture", asset_view->file_name, strlen(asset_view->file_name));
                            else ImGui::InputText("texture", na, strlen(na));
                            if (ImGui::BeginDragDropTarget()) {
                                auto* p = ImGui::AcceptDragDropPayload("asset");
                                if (p) {
                                    auto payload = (Gui_Payload*)p->Data;
                                    auto new_id = (asset_id_t)(uintptr_t)payload->value;
                                    view_request.asset_id = new_id;
                                    asset_view = get_module("asset_manager")->request<Asset*>(&view_request);
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
                        [](void* data, ImGuiContext* ctx) {
                            ImGui::SetCurrentContext(ctx);
                            ImGui::ColorEdit4("tint", (f32*)data);
                        },
                        "tint",
                        sizeof(color),
                        sizeof(asset_id_t),
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
                        [](void* data, ImGuiContext* ctx) {
                            ImGui::SetCurrentContext(ctx);
                            ImGui::ColorEdit4("clear_color", (f32*)data);
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
        return name_id_map[name];
    }

}

