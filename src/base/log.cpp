#include "log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

spdlog::logger logger::details::CreateLogger(std::string name) {
    static auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    static auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log", true);

    spdlog::logger logger(std::move(name), {stdout_sink, file_sink});
    logger.set_level(spdlog::level::trace);
    logger.flush_on(spdlog::level::trace);
    return logger;
}
