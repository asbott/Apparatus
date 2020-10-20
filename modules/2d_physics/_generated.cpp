#include "apparatus.h"
#include "D:/dev/Apparatus/modules/2d_physics/2d_physics.h"

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
            uintptr_t id = (uintptr_t)typeid(PhysicsBody2D).name();
            runtime_ids.emplace(id);
            name_id_map["PhysicsBody2D"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<PhysicsBody2D>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<PhysicsBody2D>(entity)) return (void*)NULL;
                    return (void*)&reg.get<PhysicsBody2D>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<PhysicsBody2D>(entity);
                },
            
                "PhysicsBody2D",
                id,
                true,
                sizeof(PhysicsBody2D),
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "friction",
                        sizeof(f32),
                        0,
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "density",
                        sizeof(f32),
                        sizeof(f32),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "restitution",
                        sizeof(f32),
                        sizeof(f32)+sizeof(f32),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "velocity",
                        sizeof(fvec2),
                        sizeof(f32)+sizeof(f32)+sizeof(f32),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "body_type",
                        sizeof(Physics_Body_Type),
                        sizeof(f32)+sizeof(f32)+sizeof(f32)+sizeof(fvec2),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "on_contact_begin",
                        sizeof(collision_callback_t),
                        sizeof(f32)+sizeof(f32)+sizeof(f32)+sizeof(fvec2)+sizeof(Physics_Body_Type),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "on_contact_end",
                        sizeof(collision_callback_t),
                        sizeof(f32)+sizeof(f32)+sizeof(f32)+sizeof(fvec2)+sizeof(Physics_Body_Type)+sizeof(collision_callback_t),
                    },
                }
            };
        }
        {
            uintptr_t id = (uintptr_t)typeid(CollisionShape2D).name();
            runtime_ids.emplace(id);
            name_id_map["CollisionShape2D"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<CollisionShape2D>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<CollisionShape2D>(entity)) return (void*)NULL;
                    return (void*)&reg.get<CollisionShape2D>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<CollisionShape2D>(entity);
                },
            
                "CollisionShape2D",
                id,
                true,
                sizeof(CollisionShape2D),
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            on_gui((CollisionShape2D*)data);
                        },
                        "offset",
                        sizeof(fvec2),
                        0,
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((CollisionShape2D*)data);
                        },
                        "is_trigger",
                        sizeof(bool),
                        sizeof(fvec2),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((CollisionShape2D*)data);
                        },
                        "half_extents",
                        sizeof(fvec2),
                        sizeof(fvec2)+sizeof(bool),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((CollisionShape2D*)data);
                        },
                        "shape_type",
                        sizeof(Collision_Shape_Type_2D),
                        sizeof(fvec2)+sizeof(bool)+sizeof(fvec2),
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

