#include "apparatus.h"
#include "D:/dev/Apparatus/modules/test_module/test_module.h"

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

module_scope {

    // Generated
	module_function(void) init() {

        {
            uintptr_t id = (uintptr_t)typeid(BallMovement).name();
            runtime_ids.emplace(id);
            name_id_map["BallMovement"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<BallMovement>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<BallMovement>(entity)) return (void*)NULL;
                    return (void*)&reg.get<BallMovement>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<BallMovement>(entity);
                },
            
                "BallMovement",
                id,
                false,
                sizeof(BallMovement),
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            do_gui<f32>("hforce", (f32*)data);
                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "hforce",
                        sizeof(f32),
                        ap_offsetof(BallMovement, hforce),
                    },
                    Property_Info { 
                        [](void* data) {
                            do_gui<f32>("jump_force", (f32*)data);
                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "jump_force",
                        sizeof(f32),
                        ap_offsetof(BallMovement, jump_force),
                    },
                }
            };
        }
        {
            uintptr_t id = (uintptr_t)typeid(WASDMovement).name();
            runtime_ids.emplace(id);
            name_id_map["WASDMovement"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<WASDMovement>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<WASDMovement>(entity)) return (void*)NULL;
                    return (void*)&reg.get<WASDMovement>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<WASDMovement>(entity);
                },
            
                "WASDMovement",
                id,
                false,
                sizeof(WASDMovement),
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            do_gui<float>("move_speed", (float*)data);
                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "move_speed",
                        sizeof(float),
                        ap_offsetof(WASDMovement, move_speed),
                    },
                }
            };
        }
        {
            uintptr_t id = (uintptr_t)typeid(AnimatedWASDMovement).name();
            runtime_ids.emplace(id);
            name_id_map["AnimatedWASDMovement"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<AnimatedWASDMovement>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<AnimatedWASDMovement>(entity)) return (void*)NULL;
                    return (void*)&reg.get<AnimatedWASDMovement>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<AnimatedWASDMovement>(entity);
                },
            
                "AnimatedWASDMovement",
                id,
                false,
                sizeof(AnimatedWASDMovement),
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            do_gui<float>("hspeed", (float*)data);
                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "hspeed",
                        sizeof(float),
                        ap_offsetof(AnimatedWASDMovement, hspeed),
                    },
                    Property_Info { 
                        [](void* data) {
                            do_gui<float>("vspeed", (float*)data);
                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "vspeed",
                        sizeof(float),
                        ap_offsetof(AnimatedWASDMovement, vspeed),
                    },
                    Property_Info { 
                        [](void* data) {
                            ImGui::InputAsset("walk_right", (asset_id_t*)data, "SpriteAnimation2DPreset");                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "walk_right",
                        sizeof(asset_id_t),
                        ap_offsetof(AnimatedWASDMovement, walk_right),
                    },
                    Property_Info { 
                        [](void* data) {
                            ImGui::InputAsset("walk_up", (asset_id_t*)data, "SpriteAnimation2DPreset");                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "walk_up",
                        sizeof(asset_id_t),
                        ap_offsetof(AnimatedWASDMovement, walk_up),
                    },
                    Property_Info { 
                        [](void* data) {
                            ImGui::InputAsset("walk_down", (asset_id_t*)data, "SpriteAnimation2DPreset");                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "walk_down",
                        sizeof(asset_id_t),
                        ap_offsetof(AnimatedWASDMovement, walk_down),
                    },
                }
            };
        }
        {
            uintptr_t id = (uintptr_t)typeid(FollowEntity).name();
            runtime_ids.emplace(id);
            name_id_map["FollowEntity"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<FollowEntity>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<FollowEntity>(entity)) return (void*)NULL;
                    return (void*)&reg.get<FollowEntity>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<FollowEntity>(entity);
                },
            
                "FollowEntity",
                id,
                false,
                sizeof(FollowEntity),
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            ImGui::InputEntity("target", (entt::entity*)data);
                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE | PROPERTY_FLAG_ENTITY),
                        "target",
                        sizeof(entity_t),
                        ap_offsetof(FollowEntity, target),
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
        entt::resolve<BallMovement>().reset();
        entt::resolve<WASDMovement>().reset();
        entt::resolve<AnimatedWASDMovement>().reset();
        entt::resolve<FollowEntity>().reset();
    }
}
