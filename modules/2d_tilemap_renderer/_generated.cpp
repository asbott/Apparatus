#include "apparatus.h"
#include "D:/dev/Apparatus/modules/2d_tilemap_renderer/2d_tilemap_renderer.h"

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
            uintptr_t id = (uintptr_t)typeid(TileMap2D).name();
            runtime_ids.emplace(id);
            name_id_map["TileMap2D"] = id;
            component_info[id] = {
                [](entt::registry& reg, entt::entity entity) { 
                    return &reg.emplace<TileMap2D>(entity);
                },
                [](entt::registry& reg, entt::entity entity) { 
                    if (!reg.has<TileMap2D>(entity)) return (void*)NULL;
                    return (void*)&reg.get<TileMap2D>(entity);
                }, 
                [](entt::registry& reg, entt::entity entity) { 
                    reg.remove<TileMap2D>(entity);
                },
            
                "TileMap2D",
                id,
                false,
                sizeof(TileMap2D),
                std::vector<Property_Info> {
                    Property_Info { 
                        [](void* data) {
                            ImGui::InputAsset("source_texture", (asset_id_t*)data, "Texture");                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "source_texture",
                        sizeof(asset_id_t),
                        ap_offsetof(TileMap2D, source_texture),
                    },
                    Property_Info { 
                        [](void* data) {
                            do_gui<fvec2>("tile_size", (fvec2*)data);
                        },
                        (Property_Flag)(PROPERTY_FLAG_NONE),
                        "tile_size",
                        sizeof(fvec2),
                        ap_offsetof(TileMap2D, tile_size),
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
        entt::resolve<TileMap2D>().reset();
    }
}
