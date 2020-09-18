#pragma once

#include "thread_server.h"

struct Graphics_Context;

void AP_API quit();

AP_API Thread_Server& get_thread_server();
AP_API thread_id_t get_graphics_thread();

int AP_API start(int argc, char** argv);