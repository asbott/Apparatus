#pragma once

#include "thread_server.h"

struct Component_Info {
	std::function<void*(entt::registry& reg, entt::entity entity)> create;
	std::function<void*(entt::registry& reg, entt::entity entity)> get;
	std::function<void(entt::registry& reg, entt::entity entity)> remove;

	std::string name;
	uintptr_t runtime_id;
};

struct Graphics_Context;

void AP_API quit();

AP_API Thread_Server& get_thread_server();
AP_API thread_id_t get_graphics_thread();

AP_API entt::registry& get_entity_registry();

AP_API int start(int argc, char** argv);