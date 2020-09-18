#pragma once

#include <spdlog/spdlog.h>

void AP_API init_logger();

void AP_API set_logger_level(spdlog::level::level_enum level);

std::shared_ptr<spdlog::logger> AP_API _get_spdlogger();

#ifndef _CONFIG_RELEASE

    #define log_trace(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::trace, __VA_ARGS__); }
	#define log_debug(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::debug, __VA_ARGS__); }
	#define log_info(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::info, __VA_ARGS__); }
	#define log_warn(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::warn, __VA_ARGS__); }
	#define log_error(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::err, __VA_ARGS__); }
	#define log_critical(...)	{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::critical, __VA_ARGS__); }

#else
    #define log_trace(...)
	#define log_debug(...)
	#define log_info(...)
	#define log_warn(...)
	#define log_error(...)
	#define log_critical(...)	{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::critical, __VA_ARGS__); }
#endif