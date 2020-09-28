#include "pch.h"

#include <GLFW/glfw3.h>

#include <imgui.h>

#include "dependencies.h"

void init_dependencies() {
#define init_stage if (all_good)

    bool all_good = true;

    init_stage {
        log_trace("Initializing GLFW");
        auto result = glfwInit();
        ap_assert(result, "Failed initializing glfw");

        if (!result) all_good = false;
    }

    init_stage  {
        auto version_result = IMGUI_CHECKVERSION();
        ap_assert(version_result, "Failed imgui version check");
        (void)version_result;
    }

    if (all_good) {
        log_info("Successfully initialized dependencies");
    } else {
        log_critical("A dependecy failed initializing; something will go terribly wrong");
    }
}
void update_dependencies() {
    
}
void shutdown_dependencies() {
    log_trace("Shutting down dependencies");

    glfwTerminate();

    log_info("Dependencies has been shut down");
}