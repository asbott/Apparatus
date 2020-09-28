#include "pch.h"

#include "module.h"

Module::Module(path_str_t mod_path, path_str_t mod_path_new, name_str_t str_id) {
    strcpy(this->mod_path, mod_path);
    strcpy(this->mod_path_new, mod_path_new);
    strcpy(this->str_id, str_id);
}

bool Module::load() {

    if (Path::can_open(mod_path)) {
        auto err = Path::remove(mod_path);
        ap_assert(err.value() == 0, "Remove fail: {}", err.message());
    }

    auto err = Path::copy(mod_path_new, mod_path);
    ap_assert(err.value() == 0, "Copy fail: {}", err.message());

    os_mod = os::load_module(mod_path);

    if (!os_mod) return false;

    #define _mod_load_fn(n) \
    n = (n##_t)os::load_module_function(os_mod, #n);\
    if (!n) { log_warn("Could not find function in module: " #n); n = NULL; }

    _mod_load_fn(on_load);
    _mod_load_fn(on_unload);
    _mod_load_fn(on_update);
    _mod_load_fn(on_render);
    _mod_load_fn(on_gui);

    _mod_load_fn(init);
    _mod_load_fn(get_component_info);
    _mod_load_fn(get_component_ids);
    _mod_load_fn(create_component);
    _mod_load_fn(get_component);
    _mod_load_fn(remove_component);
    _mod_load_fn(get_component_id);

    _mod_load_fn(_request);

    return true;
}

bool Module::unload() {
    if (!os_mod) return false;
    os::free_module(os_mod);
    return true;
}

bool Module::reload() {
    return this->unload() && this->load();
}