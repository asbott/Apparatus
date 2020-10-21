#include "pch.h"

#ifdef _OS_LINUX

#include <dlfcn.h>

#include "os.h"

namespace os {
	module_t load_module(str_ptr_t path) {
		module_t lib = dlopen(path, RTLD_NOW | RTLD_DEEPBIND);
		if (char* err = dlerror()) {
			log_error("Failed loading module: {}", err);
		}
		return lib;
	}
	void* load_module_function(module_t mod, str_ptr_t name_of_function) {
		return dlsym(mod, name_of_function);
	}
	void free_module(module_t mod) {
		dlclose(mod);
	}
}

#endif