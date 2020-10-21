#pragma once

struct Component_Info;
struct ImGuiContext;

#include "os.h"

#ifndef _OS_WINDOWS
    #define __cdecl 
#endif

struct Module {

    typedef void (__cdecl *on_load_t)(); 
    typedef void (__cdecl *on_unload_t)(); 
    typedef void (__cdecl *on_update_t)(float); 
    typedef void (__cdecl *on_render_t)(); 
    typedef void (__cdecl *on_gui_t)(); 

    typedef void (__cdecl *on_play_begin_t)(); 
    typedef void (__cdecl *on_play_stop_t)(); 

    typedef void (__cdecl *save_to_disk_t)  (str_ptr_t);
    typedef void (__cdecl *load_from_disk_t)(str_ptr_t);

    typedef void (__cdecl *init_t)(); 
    typedef void (__cdecl *deinit_t)(); 
    typedef Component_Info* (__cdecl *get_component_info_t)(uintptr_t); 
    typedef const Hash_Set<uintptr_t>& (__cdecl *get_component_ids_t)(); 
    typedef void* (__cdecl *create_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef void* (__cdecl *get_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef void (__cdecl *remove_component_t)(uintptr_t, entt::registry&, entt::entity); 
    typedef uintptr_t (__cdecl *get_component_id_t)(const std::string&); 

    typedef void (__cdecl *set_imgui_context_t)(ImGuiContext*);

    typedef void* (__cdecl *get_function_library_t)();

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
    deinit_t deinit = NULL;
    get_component_info_t get_component_info = NULL;
    get_component_ids_t  get_component_ids  = NULL;
    create_component_t   create_component   = NULL;
    get_component_t      get_component      = NULL;
    remove_component_t   remove_component   = NULL;
    get_component_id_t   get_component_id   = NULL;

    set_imgui_context_t  set_imgui_context = NULL;

    bool is_loaded = false;

    get_function_library_t get_function_library;

    Module(str_ptr_t mod_path, str_ptr_t mod_path_new, str_ptr_t str_id = "unnamed");

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


