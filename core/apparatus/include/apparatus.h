#pragma once

#include "pch.h"

#include "start.h"

#include "graphics/directx11.h"
#include "graphics/opengl45.h"

#include "render_context.h"

#include "input_codes.h"

#include "thread_server.h"

#include "image_import.h"

using namespace mz;

#ifdef _OS_WINDOWS
    #define module_function(ret) extern "C" __declspec(dllexport) ret __cdecl
    #define module_scope namespace
#elif defined(_OS_LINUX)
    #define module_function(ret) extern "C" ret
    #define module_scope namespace 
#endif

#define tag(...) 