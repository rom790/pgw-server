#include "http/http_server.h"
#include <stdexcept>

HttpServer::HttpServer(int port, const std::string& host) 
    : port_(port), host_(host) {
    
    server_ = std::make_unique<httplib::Server>();
    
    // Настройка стандартных параметров сервера
    server_->set_keep_alive_max_count(100);
    // 5 секунд на ожидания получения и отправки
    server_->set_read_timeout(5, 0);  
    server_->set_write_timeout(5, 0); 

    server_->set_logger([](const auto& req, const auto& res) {
        Logger::debug("HTTP {} {} -> {}", req.method, req.path, res.status);
    });
    
    setup_routes();
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    if (running_) return;
    
    running_ = true;
    server_thread_ = std::thread([this]() {
        Logger::info("Starting HTTP server on {}:{}", host_, port_);
        if (!server_->listen(host_.c_str(), port_)) {
            Logger::error("HTTP server failed to start on port {}", port_);
        }
    });
}

void HttpServer::stop() {
    if (!running_) return;
    
    running_ = false;
    server_->stop();
    
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    Logger::info("HTTP server stopped");
}

bool HttpServer::is_running() const noexcept{
    return running_;
}

void HttpServer::add_get_handler(std::string_view path, RequestHandler handler) {
    server_->Get(std::string(path), [this, handler](const httplib::Request& req, httplib::Response& res) {
        ++request_count_;
        handler(req, res);
    });
}


void HttpServer::setup_routes() {
    server_->Get("/health", [this](const auto& req, auto& res) {
        handle_health_check(req, res);
    });
}

void HttpServer::handle_health_check(const httplib::Request&, httplib::Response& res) {
    res.set_content(R"({"status":"ok"})", "application/json");
}