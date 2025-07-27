#pragma once
#include <functional>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <mutex>
#include <arpa/inet.h>
#include <string_view>

class UdpServer {
public:
    using MessageHandler = std::function<void(const std::string&, const sockaddr_in&)>;
    
    UdpServer(std::string_view ip, int port, MessageHandler handler);
    ~UdpServer();
    
    bool start();
    void stop();
    bool is_running() const;
    
    void send(std::string_view message, const sockaddr_in& addr);

private:
    void worker_thread();
    bool setup_socket();
    bool setup_epoll();
    void handle_events();
    
    int sockfd_ = -1;
    int epoll_fd_ = -1;
    std::string ip_;
    int port_;
    std::atomic<bool> running_{false};
    std::thread worker_thread_;
    std::mutex send_mutex_;
    MessageHandler message_handler_;


    struct sockaddr_in client_addr_;
    char buffer_[65536]; // Максимальный размер UDP пакета
};