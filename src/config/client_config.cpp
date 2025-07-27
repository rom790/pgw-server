#include "config/client_config.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "utils/logger.h"

ClientConfig::ClientConfig(std::string_view config_file_path) {
    load_config(config_file_path);
    validate_config();
}

void ClientConfig::load_config(std::string_view config_file_path) {
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

    server_ip_ = config.value("server_ip", server_ip_);
    server_port_ = config.value("server_port", server_port_);
    log_file_ = config.value("log_file", log_file_);
    log_level_ = config.value("log_level", log_level_);
    console_output_ = config.value("console_output", console_output_);
}


void ClientConfig::validate_config() {

    if (server_ip_.empty()) {
        throw std::runtime_error("Server IP cannot be empty");
    }
    
    if (server_port_ <= 0 || server_port_ > 65535) {
        throw std::runtime_error("Invalid server port number");
    }
    
    const std::vector<std::string> allowed_log_levels = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"
    };

    if (std::find(allowed_log_levels.begin(), allowed_log_levels.end(), log_level_) == allowed_log_levels.end()) {
        throw std::runtime_error("Invalid log level");
    }
}