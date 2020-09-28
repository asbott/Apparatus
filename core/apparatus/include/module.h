#pragma once

struct Component_Info;

#include "graphics/graphics_api.h"

#include "os.h"

struct Module {

    typedef void (__cdecl *on_load_t)(Graphics_Context*); 
    typedef void (__cdecl *on_unload_t)(Graphics_Context*); 
    typedef void (__cdecl *on_update_t)(float); 
    typedef void (__cdecl *on_render_t)(Graphics_Context*); 
    typedef void (__cdecl *on_gui_t)(Graphics_Context*, ImGuiContext*); 

    typedef void (__cdecl *init_t)(); 
    typedef Component_Info* (__cdecl *get_component_info_t)(uintptr_t); 
    typedef const Hash_Set<uintptr_t>& (__cdecl *get_component_ids_t)(); 
    typedef void* (__cdecl *create_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef void* (__cdecl *get_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef void (__cdecl *remove_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef uintptr_t (__cdecl *get_component_id_t)(const std::string&); 

    typedef void* (__cdecl *_request_t)(void* ud);

    on_load_t on_load = NULL;
    on_unload_t on_unload = NULL;
    on_update_t on_update = NULL;
    on_render_t on_render = NULL;
    on_gui_t on_gui = NULL;

    init_t init = NULL;
    get_component_info_t get_component_info = NULL;
    get_component_ids_t get_component_ids = NULL;
    create_component_t create_component = NULL;
    get_component_t get_component = NULL;
    remove_component_t remove_component = NULL;
    get_component_id_t get_component_id = NULL;

    _request_t _request = NULL;

    Module(path_str_t mod_path, path_str_t mod_path_new, name_str_t str_id = "unnamed");

    template <typename type_t>
    inline type_t request(void* ud) {
        if (this->_request) {
            return (type_t)_request(ud);
        }
        return 0;
    }

    bool load();
    bool unload();
    bool reload();

    os::module_t os_mod = NULL;
    path_str_t mod_path, mod_path_new;
    name_str_t str_id;
};


