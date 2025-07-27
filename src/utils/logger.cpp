#include "utils/logger.h"
#include <stdexcept>

std::shared_ptr<spdlog::logger> Logger::logger_;

void Logger::init(
    const std::string& log_file,
    const std::string& logger_name,
    const std::string& log_level,
    bool console_output
) {
    try {
        std::vector<spdlog::sink_ptr> sinks;

        if (console_output) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
            sinks.push_back(console_sink);
        }

        if (!log_file.empty()) {
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, false);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");
            sinks.push_back(file_sink);
        }
        
        if (sinks.empty()) {
            throw std::runtime_error("No log sinks configured");
        }

        logger_ = std::make_shared<spdlog::logger>(logger_name, sinks.begin(), sinks.end());

        logger_->set_level(parse_level(log_level));
        logger_->flush_on(spdlog::level::trace);

        spdlog::register_logger(logger_);
        
    } catch (const spdlog::spdlog_ex& ex) {
        throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
    }
}

spdlog::level::level_enum Logger::parse_level(const std::string& level) {
    if (level == "TRACE") return spdlog::level::trace; 
    if (level == "DEBUG") return spdlog::level::debug;
    if (level == "INFO") return spdlog::level::info;
    if (level == "WARN") return spdlog::level::warn;
    if (level == "ERROR") return spdlog::level::err;
    if (level == "CRITICAL") return spdlog::level::critical;
    if (level == "OFF") return spdlog::level::off;
    
    throw std::invalid_argument("Unknown log level: " + level);
}

std::shared_ptr<spdlog::logger> Logger::create_null_logger() {
    auto logger = std::make_shared<spdlog::logger>("null_logger");
    logger->set_level(spdlog::level::off);
    return logger;
}