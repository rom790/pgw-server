#pragma once
#include "config/server_config.h"
#include "session/session_manager.h"
#include "network/udp_server.h"
#include "http/http_server.h"
#include "cdr/cdr_manager.h"
#include "utils/bcd_converter.h"
#include <memory>
#include <atomic>
#include <csignal>

class PgwServer {
public:
    explicit PgwServer() = default;
    void init(const std::string& config_file);
    void run();
    
    // Запрещаем копирование и присваивание
    PgwServer(const PgwServer&) = delete;
    PgwServer& operator=(const PgwServer&) = delete;

private:
    // Конфигурация сервера
    std::unique_ptr<ServerConfig> config_;
    
    // Основные компоненты
    std::shared_ptr<CdrManager> cdr_manager_;
    std::unique_ptr<SessionManager> session_manager_;
    std::unique_ptr<UdpServer> udp_server_;
    std::unique_ptr<HttpServer> http_server_;

    void setup_http_server();

    void handle_udp_message(const std::string& message, const sockaddr_in& client_addr);

    void send_udp_response(const std::string& response, const sockaddr_in& addr);
};

// Объявление глобального флага для обработки сигналов
extern std::atomic<bool> shutdown_flag;