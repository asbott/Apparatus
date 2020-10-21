#include "pch.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/sinks/dist_sink.h"

#include <spdlog/fmt/ostr.h>

#include "logger.h"

#include <spdlog/sinks/base_sink.h>

#include "start.h"

template<typename Mutex>
class imgui_sink : public spdlog::sinks::base_sink <Mutex>
{
public:
    imgui_sink(Log_Context* ctx) : ctx(ctx) {}
protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        ctx->entries.push_back({ fmt::to_string(formatted), msg.level });
        ctx->has_new = true;
    }

    void flush_() override 
    {
        ctx->entries.clear();
    }

    Log_Context* ctx;
};

void Log_Context::do_gui(Gui_Window* wnd) {
    ImGui::DoGuiWindow(wnd, [this]() {
        ImGui::BeginMenuBar();
        if (ImGui::BeginMenu("Settings")) {
            ImGui::RDragInt("Limit", &log_entry_limit, 1.f, 1, 100000);

            ImGui::Separator();
            ImGui::Text("Colors");
            for (auto& [level, color] : filter_colors) {
                ImGui::ColorEdit4(spdlog::level::to_string_view(level).data(), color.ptr);
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Filter")) {
            for (spdlog::level::level_enum level = spdlog::level::trace; level <= spdlog::level::critical; level = (spdlog::level::level_enum)((s32)level + 1)) {
                if (level % 3 != 0) ImGui::SameLine();
                ImGui::RCheckbox(spdlog::level::to_string_view(level).data(), &filter_flags[level]);
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
        while (entries.size() > log_entry_limit) {
            entries.pop_front();
        }
        for (auto& entry : entries) {
            if (filter_flags[entry.level]) {
                ImGui::TextColored(filter_colors[entry.level], "%s", entry.dynamic_str.c_str());
            }
        }
        if (has_new) {
            ImGui::SetScrollHereY();
            has_new = false;
        }
    }, ImGuiWindowFlags_MenuBar);
}

#include "spdlog/details/null_mutex.h"
#include <mutex>
using imgui_sink_mt = imgui_sink<std::mutex>;
using imgui_sink_st = imgui_sink<spdlog::details::null_mutex>;

std::shared_ptr<spdlog::logger> spdlogger;

void init_logger(std::ostream& ostr, Log_Context* ctx) {
    #ifdef _CONFIG_DEBUG
		spdlog::set_pattern("%^================================================================================================================\n[%n - %l - %H:%M:%S:%e - %s - %! - Line %# - Thread %t]\n================================================================================================================%$\n%v\n");
    #elif defined(_CONFIG_TEST) || defined(_CONFIG_RELEASE)
    spdlog::set_pattern("%^%v%$\n");
    #else
        #error No configuration defined
    #endif

    auto ostr_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(ostr, true);
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto gui_sink = std::make_shared<imgui_sink_mt>(ctx);

    std::vector<spdlog::sink_ptr> sinks = { ostr_sink, stdout_sink, gui_sink };
       
    spdlogger = spdlog::default_factory::create<spdlog::sinks::dist_sink_mt>("apparatus", sinks);

    log_info("The logger has been initialized!");
}

std::shared_ptr<spdlog::logger> _get_spdlogger() {
    return spdlogger;
}

void set_logger_level(spdlog::level::level_enum level) {
    spdlogger->set_level(level);
}