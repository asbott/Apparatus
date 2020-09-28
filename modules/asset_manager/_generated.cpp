#include "apparatus.h"

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

