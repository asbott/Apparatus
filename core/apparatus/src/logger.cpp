#include "pch.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/fmt/ostr.h>

#include "logger.h"

std::shared_ptr<spdlog::logger> spdlogger;

void init_logger() {
    #ifdef _CONFIG_DEBUG
		spdlog::set_pattern("%^================================================================================================================\n[%n - %l - %H:%M:%S:%e - %s - %! - Line %# - Thread %t]\n================================================================================================================%$\n%v\n");
    #elif defined(_CONFIG_TEST) || defined(_CONFIG_RELEASE)
    spdlog::set_pattern("%^%v%$\n");
    #else
        #error No configuration defined
    #endif

    spdlogger = spdlog::stdout_color_mt("Core");

    log_info("The logger has been initialized!");
}

std::shared_ptr<spdlog::logger> _get_spdlogger() {
    return spdlogger;
}

void set_logger_level(spdlog::level::level_enum level) {
    spdlogger->set_level(level);
}