#pragma once

struct Component_Info;

#include "graphics/graphics_api.h"

#include "os.h"

struct Module {

    typedef void (__cdecl *on_load_t)(Graphics_Context*); 
    typedef void (__cdecl *on_unload_t)(Graphics_Context*); 
    typedef void (__cdecl *on_update_t)(float); 
    typedef void (__cdecl *on_render_t)(Graphics_Context*); 
    typedef void (__cdecl *on_gui_t)(Graphics_Context*); 

    typedef void (__cdecl *on_play_begin_t)(); 
    typedef void (__cdecl *on_play_stop_t)(); 

    typedef void (__cdecl *save_to_disk_t)  (str_ptr_t);
    typedef void (__cdecl *load_from_disk_t)(str_ptr_t);

    typedef void (__cdecl *init_t)(); 
    typedef Component_Info* (__cdecl *get_component_info_t)(uintptr_t); 
    typedef const Hash_Set<uintptr_t>& (__cdecl *get_component_ids_t)(); 
    typedef void* (__cdecl *create_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef void* (__cdecl *get_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef void (__cdecl *remove_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef uintptr_t (__cdecl *get_component_id_t)(const std::string&); 

    typedef void (__cdecl *set_imgui_context_t)(ImGuiContext*);

    typedef void* (__cdecl *_request_t)(void* ud);

    on_load_t   on_load   = NULL;
    on_unload_t on_unload = NULL;
    on_update_t on_update = NULL;
    on_render_t on_render = NULL;
    on_gui_t    on_gui    = NULL;

    on_play_begin_t on_play_begin = NULL;
    on_play_stop_t  on_play_stop  = NULL;

    save_to_disk_t   save_to_disk   = NULL;
    load_from_disk_t load_from_disk = NULL;

    init_t init = NULL;
    get_component_info_t get_component_info = NULL;
    get_component_ids_t  get_component_ids  = NULL;
    create_component_t   create_component   = NULL;
    get_component_t      get_component      = NULL;
    remove_component_t   remove_component   = NULL;
    get_component_id_t   get_component_id   = NULL;

    set_imgui_context_t  set_imgui_context = NULL;

    bool is_loaded = false;

    _request_t _request = NULL;

    Module(path_str_t mod_path, path_str_t mod_path_new, name_str_t str_id = "unnamed");

    template <typename ret_type_t, typename req_type_t>
    inline ret_type_t request(req_type_t& req) {
        if (this->_request) {
            return (ret_type_t)_request(&req);
        }
        if constexpr (!std::is_same<ret_type_t, void>())
            return 0;
    }

    inline bool has_component(uintptr_t comp_id, entt::registry& reg, entt::entity entity) {
        return this->get_component(comp_id, reg, entity) != NULL;
    }

    bool load();
    bool unload();
    bool reload();

    os::module_t os_mod = NULL;
    path_str_t mod_path, mod_path_new;
    name_str_t str_id;
};


