#include "apparatus.h"
#include "D:/dev/Apparatus/test_module/test.h"

#include <vector>
#include <functional>

Hash_Set<uintptr_t> runtime_ids;
Hash_Map<std::string, uintptr_t> name_id_map;
Hash_Map<uintptr_t, Component_Info> component_info;

extern "C" {

    // Generated
	__declspec(dllexport) void __cdecl init() {

        {
            uintptr_t id = (uintptr_t)typeid(Transform).name();
            runtime_ids.emplace(id);
            name_id_map["Transform"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<Transform>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.get<Transform>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<Transform>(entity);
                },
            
                "Transform",
                id
            };
        }
        {
            uintptr_t id = (uintptr_t)typeid(SpriteComponent).name();
            runtime_ids.emplace(id);
            name_id_map["SpriteComponent"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<SpriteComponent>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.get<SpriteComponent>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<SpriteComponent>(entity);
                },
            
                "SpriteComponent",
                id
            };
        }
        {
            uintptr_t id = (uintptr_t)typeid(AnotherComponent).name();
            runtime_ids.emplace(id);
            name_id_map["AnotherComponent"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<AnotherComponent>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.get<AnotherComponent>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<AnotherComponent>(entity);
                },
            
                "AnotherComponent",
                id
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

    __declspec(dllexport) void __cdecl remove_component(uintptr_t runtime_id, entt::registry& reg, entt::entity entity) {
		component_info[runtime_id].remove(reg, entity);
	}

    __declspec(dllexport) uintptr_t __cdecl get_component_id(const std::string& name) {
        return name_id_map[name];
    }

}

