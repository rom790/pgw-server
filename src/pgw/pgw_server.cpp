#include <csignal>
#include <thread>
#include <filesystem>
#include <chrono>
#include <unistd.h>
#include "pgw/pgw_server.h"

std::atomic<bool> shutdown_flag{false};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        shutdown_flag.store(true);
    }
}

void PgwServer::init(const std::string& config_file) {

    if (config_file.empty()) {
        throw std::runtime_error("Config file path cannot be empty");
    }

    if (!std::filesystem::exists(config_file)) {
        throw std::runtime_error("Config file not found: " + config_file);
    }
    if (!std::filesystem::is_regular_file(config_file)) {
        throw std::runtime_error("Config file is not a regular file: " + config_file);
    }

    config_ = std::make_unique<ServerConfig>(config_file);

    if (!config_->get_log_file().empty()) {
        Logger::init(config_->get_log_file(), "server_logger",  config_->get_log_level(), config_->get_console_output());
    }
    Logger::get_logger()->info("=== New process started (PID: {}) ===", ::getpid());

    cdr_manager_ = std::make_shared<CdrManager>(
        config_->get_cdr_file());

    session_manager_ = std::make_unique<SessionManager>(
        cdr_manager_,
        config_->get_session_timeout_sec(),
        config_->get_blacklist());

    udp_server_ = std::make_unique<UdpServer>(
        config_->get_udp_ip(),
        config_->get_udp_port(),
        [this](const std::string& msg, const sockaddr_in& addr) {
            handle_udp_message(msg, addr);
        });


    http_server_ = std::make_unique<HttpServer>(config_->get_http_port());
    setup_http_server();
}

void PgwServer::run() {
    // Регистрация обработчиков сигналов для остановки сревера 
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    udp_server_->start();
    http_server_->start();
    
    // Поток для очистки устаревших сессий
    std::thread cleanup_thread([this]() {
        while (!shutdown_flag) {
            session_manager_->cleanup_expired_sessions();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
    
    Logger::get_logger()->info("PGW Server started successfully");

    while (!shutdown_flag) {
        std::this_thread::yield();
    }

    Logger::get_logger()->info("Shutting down server...");
    cleanup_thread.join();

    session_manager_->graceful_shutdown(config_->get_graceful_shutdown_rate());
    http_server_->stop();
    udp_server_->stop();
}

void PgwServer::setup_http_server() {

    http_server_->add_get_handler("/check_subscriber", 
        [this](const httplib::Request& req, httplib::Response& res) {
            if (!req.has_param("imsi")) {
                res.status = 400;
                res.set_content("IMSI parameter missing", "text/plain");
                return;
            }
            
            std::string imsi = req.get_param_value("imsi");
            bool active = session_manager_->session_exists(imsi);
            res.set_content(active ? "active" : "not active", "text/plain");
        });
    

    http_server_->add_get_handler("/stop", 
        [this](const httplib::Request&, httplib::Response& res) {
            res.set_content("Shutting down server...", "text/plain");
            shutdown_flag.store(true);
        });
}

void PgwServer::handle_udp_message(const std::string& message, const sockaddr_in& client_addr) {
    try {

        std::string imsi = BCDConverter::bcd_to_imsi(
            std::vector<uint8_t>(message.begin(), message.end()));
            
            if (!BCDConverter::validate_imsi(imsi)) {
                Logger::get_logger()->warn("Invalid IMSI received");
                send_udp_response("rejected", client_addr);
                return;
            }
            
        bool created = session_manager_->create_session(imsi);
        std::string response = created ? "created" : "rejected";

        send_udp_response(response, client_addr);
        
    } catch (const std::exception& e) {
        Logger::get_logger()->error("Message processing error: {}", e.what());
        send_udp_response("error", client_addr);
    }
}

void PgwServer::send_udp_response(const std::string& response, const sockaddr_in& addr) {
    udp_server_->send(response, addr);
}

