#pragma once
#include <memory>
#include <string>
#include <functional>
#include <string_view>
#include <httplib.h>
#include "utils/logger.h"

class HttpServer {
public:
    using RequestHandler = std::function<void(const httplib::Request&, httplib::Response&)>;
    
    HttpServer(int port, const std::string& host = "0.0.0.0");
    ~HttpServer();
    
    void start();
    void stop();
    bool is_running() const noexcept;
    
    // Регистрация endpoint'ов
    void add_get_handler(std::string_view path, RequestHandler handler);


private:
    void setup_routes();
    
    std::unique_ptr<httplib::Server> server_;
    std::thread server_thread_;
    std::atomic<bool> running_{false};
    std::atomic<size_t> request_count_{0};
    int port_;
    std::string host_;
    
    // Встроенные обработчики
    void handle_health_check(const httplib::Request&, httplib::Response& res);
};