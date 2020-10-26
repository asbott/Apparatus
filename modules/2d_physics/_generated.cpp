#include "apparatus.h"
#include "./2d_physics.h"

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
        ImGui::RDragFloat2(label.c_str(), (f32*)data, 0.1f);
    } else if constexpr (std::is_same<type_t, mz::fvec3>()) {
        ImGui::RDragFloat3(label.c_str(), (f32*)data, 0.1f);
    } else if constexpr (std::is_same<type_t, mz::fvec4>()) {
        ImGui::RDragFloat4(label.c_str(), (f32*)data, 0.1f);
    } else {
        ImGui::Text("%s N/A", label.c_str());
    }
}

module_scope {

    // Generated
	module_function(void) init() {

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
                        ap_offsetof(PhysicsBody2D, friction),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "density",
                        sizeof(f32),
                        ap_offsetof(PhysicsBody2D, density),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "restitution",
                        sizeof(f32),
                        ap_offsetof(PhysicsBody2D, restitution),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "velocity",
                        sizeof(fvec2),
                        ap_offsetof(PhysicsBody2D, velocity),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "body_type",
                        sizeof(Physics_Body_Type),
                        ap_offsetof(PhysicsBody2D, body_type),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "on_contact_begin",
                        sizeof(collision_callback_t),
                        ap_offsetof(PhysicsBody2D, on_contact_begin),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((PhysicsBody2D*)data);
                        },
                        "on_contact_end",
                        sizeof(collision_callback_t),
                        ap_offsetof(PhysicsBody2D, on_contact_end),
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
                        ap_offsetof(CollisionShape2D, offset),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((CollisionShape2D*)data);
                        },
                        "is_trigger",
                        sizeof(bool),
                        ap_offsetof(CollisionShape2D, is_trigger),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((CollisionShape2D*)data);
                        },
                        "half_extents",
                        sizeof(fvec2),
                        ap_offsetof(CollisionShape2D, half_extents),
                    },
                    Property_Info { 
                        [](void* data) {
                            on_gui((CollisionShape2D*)data);
                        },
                        "shape_type",
                        sizeof(Collision_Shape_Type_2D),
                        ap_offsetof(CollisionShape2D, shape_type),
                    },
                }
            };
        }

    
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

module_scope {
    module_function(void) deinit() {
        entt::resolve<PhysicsBody2D>().reset();
        entt::resolve<CollisionShape2D>().reset();
    }
}
