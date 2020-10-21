#pragma once

namespace os {
	typedef void* module_t;

	module_t load_module(str_ptr_t path);
	void* load_module_function(module_t mod, str_ptr_t name_of_function);
	void free_module(module_t mod);
}