#pragma once
#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
class Logger {
public:
    static void init(
        const std::string& log_file, 
        const std::string& logger_name,
        const std::string& log_level = "OFF",
        bool console_output = true
    );

    static std::shared_ptr<spdlog::logger> get_logger() {
        if (!logger_) {
            static auto null_logger = create_null_logger();
            return null_logger;
        }
        return logger_;
    }

    template<typename... Args>
    static void debug(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) logger_->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) logger_->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) logger_->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) logger_->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void critical(spdlog::format_string_t<Args...> fmt, Args&&... args) {
        if (logger_) logger_->critical(fmt, std::forward<Args>(args)...);
    }

    static void flush() {
        if (logger_) logger_->flush();
    }

private:
    static std::shared_ptr<spdlog::logger> logger_;
    static std::shared_ptr<spdlog::logger> create_null_logger();    
    static spdlog::level::level_enum parse_level(const std::string& level);
};