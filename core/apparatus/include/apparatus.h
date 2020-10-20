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

#define _export __declspec(dllexport)
#define tag(...) 