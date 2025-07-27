#include "config/server_config.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <filesystem>

#include "utils/logger.h"

ServerConfig::ServerConfig(std::string_view config_file_path) {
    load_config(config_file_path);
    validate_config();
}


void ServerConfig::load_config(std::string_view config_file_path) {
    if (!std::filesystem::exists(config_file_path)) {
        throw std::runtime_error("Config file does not exist: " + std::string(config_file_path));
    }

    std::ifstream config_file(config_file_path.data());
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file: " + std::string(config_file_path));
    }

    nlohmann::json config;
    try {
        config_file >> config;
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    }

    udp_ip_ = config.value("udp_ip", udp_ip_);
    udp_port_ = config.value("udp_port", udp_port_);
    session_timeout_sec_ = config.value("session_timeout_sec", session_timeout_sec_);
    cdr_file_ = config.value("cdr_file", cdr_file_);
    http_port_ = config.value("http_port", http_port_);
    graceful_shutdown_rate_ = config.value("graceful_shutdown_rate", graceful_shutdown_rate_);
    log_file_ = config.value("log_file", log_file_);
    log_level_ = config.value("log_level", log_level_);
    console_output_ = config.value("console_output", console_output_);

    // Загрузка blacklist
    if (config.contains("blacklist") && config["blacklist"].is_array()) {
        for (const auto& item : config["blacklist"]) {
            blacklist_.push_back(item.get<std::string>());
        }
    }
}

void ServerConfig::validate_config() {
    if (udp_ip_.empty()) {
        throw std::runtime_error("UDP IP cannot be empty");
    }

    auto validate_port = [](int port, const std::string& name) {
        if (port <= 0 || port > 65535) {
            throw std::runtime_error("Invalid " + name + " port number");
        }
    };

    validate_port(udp_port_, "UDP");
    validate_port(http_port_, "HTTP");

    if (session_timeout_sec_ <= 0) {
        throw std::runtime_error("Session timeout must be positive");
    }

    if (graceful_shutdown_rate_ < 0) {
        throw std::runtime_error("Graceful shutdown rate cannot be negative");
    }

    constexpr std::array allowed_log_levels = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"
    };
    
    if (std::find(allowed_log_levels.begin(), allowed_log_levels.end(), log_level_) == allowed_log_levels.end()) {
        throw std::runtime_error("Invalid log level");
    }

    // Валидация blacklist
    for (const auto& imsi : blacklist_) {
        if (imsi.empty() || imsi.length() > 15 || 
            !std::all_of(imsi.begin(), imsi.end(), ::isdigit)) {
            throw std::runtime_error("Invalid IMSI in blacklist: " + imsi);
        }
    }
}