#include "apparatus.h"
#include "D:/dev/Apparatus/modules/2d_sprite_renderer/2d_sprite_renderer.h"

#include <vector>
#include <functional>

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
                sizeof(Sprite2D),
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            ImGui::InputAsset("texture", (asset_id_t*)data, "Texture");                        },
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
                    Property_Info { 
                        [](void* data) {
                            do_gui<int>("depth_level", (int*)data);
                        },
                        "depth_level",
                        sizeof(int),
                        sizeof(asset_id_t)+sizeof(color)+sizeof(fvec2),
                    },
                }
            };
        }
        {
            uintptr_t id = (uintptr_t)typeid(SpriteAnimation2D).name();
            runtime_ids.emplace(id);
            name_id_map["SpriteAnimation2D"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<SpriteAnimation2D>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<SpriteAnimation2D>(entity)) return (void*)NULL;
                    return (void*)&reg.get<SpriteAnimation2D>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<SpriteAnimation2D>(entity);
                },
            
                "SpriteAnimation2D",
                id,
                false,
                sizeof(SpriteAnimation2D),
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            do_gui<bool>("use_asset", (bool*)data);
                        },
                        "use_asset",
                        sizeof(bool),
                        0,
                    },
                    Property_Info { 
                        [](void* data) {
                            ImGui::InputAsset("anim_asset", (asset_id_t*)data, "2");                        },
                        "anim_asset",
                        sizeof(asset_id_t),
                        sizeof(bool),
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

