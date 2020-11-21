#pragma once

#include <spdlog/spdlog.h>

struct Gui_Window;

struct Log_Context {
	struct Log_Entry {
		Dynamic_String dynamic_str;
		spdlog::level::level_enum level;
	};

	void do_gui(Gui_Window* wnd);

	Deque<Log_Entry> entries;
	s32 log_entry_limit = 100;
	bool has_new = true;
	Hash_Map<spdlog::level::level_enum, bool> filter_flags = {
		{ spdlog::level::trace,    true },
		{ spdlog::level::debug,    true },
		{ spdlog::level::info,     true },
		{ spdlog::level::warn,     true },
		{ spdlog::level::err,      true },
		{ spdlog::level::critical, true }
	};
	Hash_Map<spdlog::level::level_enum, mz::color> filter_colors = {
		{ spdlog::level::trace,    mz::COLOR_WHITE },
		{ spdlog::level::debug,    mz::COLOR_CYAN },
		{ spdlog::level::info,     mz::COLOR_GREEN },
		{ spdlog::level::warn,     mz::COLOR_YELLOW },
		{ spdlog::level::err,      mz::COLOR_ORANGE },
		{ spdlog::level::critical, mz::COLOR_RED }
	};
};

void AP_API init_logger(std::ostream& ostr, Log_Context* ctx);

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
    #define log_trace(...) (void)0
	#define log_debug(...) (void)0
	#define log_info(...) (void)0
	#define log_warn(...) (void)0
	#define log_error(...) (void)0
	#define log_critical(...)	{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::critical, __VA_ARGS__); }
#endif