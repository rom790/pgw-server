#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <string_view>

class ServerConfig {
public:
    ServerConfig(std::string_view ServerConfig_file);
    
    const std::string& get_udp_ip() const noexcept{ return udp_ip_; }
    const std::string& get_cdr_file() const noexcept{ return cdr_file_; }
    const std::string& get_log_level() const noexcept{ return log_level_; }
    const std::string& get_log_file() const noexcept{ return log_file_; }
    
    int get_udp_port() const noexcept{ return udp_port_; }
    int get_session_timeout_sec() const noexcept{ return session_timeout_sec_; }
    int get_http_port() const noexcept{ return http_port_; }
    int get_graceful_shutdown_rate() const noexcept{ return graceful_shutdown_rate_; }
    
    bool get_console_output() const noexcept { return console_output_; }
    
    const std::vector<std::string>& get_blacklist() const noexcept{ return blacklist_; }
    
    bool is_valid() const noexcept { return is_valid_; }
    
private:
    std::string udp_ip_;
    int udp_port_;
    int session_timeout_sec_;
    std::string cdr_file_;
    int http_port_;
    int graceful_shutdown_rate_;
    std::string log_file_;
    std::string log_level_;
    bool console_output_ = false;
    std::vector<std::string> blacklist_;

    bool is_valid_ = false;

    void load_config(std::string_view ServerConfig_file);
    void validate_config();
};