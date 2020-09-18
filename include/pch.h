#ifdef _MSC_VER
    #pragma warning(disable : 4201)
#endif

#include <stdio.h>
#include <stdarg.h>

#include <functional>
#include <optional>

#include <spdlog/fmt/ostr.h>

#include <mz_vector.hpp>
#include <mz_matrix.hpp>

#include "constant_limits.h"

#include "common.h"

#include "base_types.h"

#include "logger.h"

#include "containers.h"

#include "file_management.h"

AP_API const char* get_executable_path();
AP_API void ___set_executable_path(const char* path);

AP_API const char* get_executable_directory();