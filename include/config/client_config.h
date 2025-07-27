#pragma once

#include <string>
#include <string_view>
#include <nlohmann/json.hpp>

class ClientConfig {
public:
    explicit ClientConfig(std::string_view config_file_path);

    const std::string& get_server_ip() const noexcept { return server_ip_; }
    int get_server_port() const noexcept { return server_port_; }

    const std::string& get_log_file() const noexcept { return log_file_; }
    const std::string& get_log_level() const noexcept { return log_level_; }
    bool get_console_output() const noexcept { return console_output_; }

private:
    void load_config(std::string_view config_file_path);
    void validate_config();

    std::string server_ip_;
    int server_port_ = 0;
    std::string log_file_;
    std::string log_level_ = "INFO";
    bool console_output_ = false;
};