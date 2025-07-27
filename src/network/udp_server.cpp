#include "network/udp_server.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include <system_error>
#include "utils/logger.h"

UdpServer::UdpServer(std::string_view ip, int port, MessageHandler handler)
    : ip_(ip), port_(port), message_handler_(std::move(handler)) {
    
    if (!setup_socket() || !setup_epoll()) {
        throw std::runtime_error("Failed to initialize UDP server");
    }
}

UdpServer::~UdpServer() {
    stop();
    if (sockfd_ != -1) close(sockfd_);
    if (epoll_fd_ != -1) close(epoll_fd_);
}

bool UdpServer::start() {
    if (running_) return true;
    
    running_ = true;
    worker_thread_ = std::thread(&UdpServer::worker_thread, this);
    
    Logger::get_logger()->info("UDP server started on {}:{}", ip_, port_);
    return true;
}

void UdpServer::stop() {
    if (!running_) return;
    
    running_ = false;
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }

    Logger::get_logger()->info("UDP server stopped.");
}

bool UdpServer::is_running() const {
    return running_;
}


void UdpServer::worker_thread() {
    const int max_events = 256;
    struct epoll_event events[max_events];
    
    while (running_) {
        int num_events = epoll_wait(epoll_fd_, events, max_events, 10); // 10ms timeout
        
        if (num_events < 0) {
            if (errno == EINTR) continue;
            Logger::get_logger()->error("epoll_wait error: {}", strerror(errno));
            break;
        }
        
        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == sockfd_) {
                handle_events();
                Logger::get_logger()->info("Server recieved data");
            }
        }
    }
}

bool UdpServer::setup_socket() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (sockfd_ < 0) {
        Logger::get_logger()->critical("Socket creation failed: {}", strerror(errno));
        return false;
    }
    
    // Настройка адреса сервера
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    
    if (inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr) <= 0) {
        Logger::get_logger()->critical("Invalid IP address: {}", ip_);
        close(sockfd_);
        sockfd_ = -1;
        return false;
    }
    
    if (bind(sockfd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        Logger::get_logger()->critical("Bind failed: {}", strerror(errno));
        close(sockfd_);
        sockfd_ = -1;
        return false;
    }

    int recv_buf_size = 1024 * 1024; 
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, sizeof(recv_buf_size));
    
    Logger::get_logger()->debug("UDP socket configured successfully");
    return true;
}

bool UdpServer::setup_epoll() {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        Logger::get_logger()->critical("epoll_create1 failed: {}", strerror(errno));
        return false;
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = sockfd_;
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sockfd_, &event) < 0) {
        Logger::get_logger()->critical("Epoll_ctl failed: {}", strerror(errno));
        close(epoll_fd_);
        epoll_fd_ = -1;
        return false;
    }
    
    Logger::get_logger()->debug("Epoll configured successfully");
    return true;
}

void UdpServer::handle_events() {
    socklen_t addr_len = sizeof(client_addr_);

    while (running_) {
        ssize_t bytes_received = recvfrom(sockfd_, buffer_, sizeof(buffer_), 0,
                                        (struct sockaddr*)&client_addr_, &addr_len);
        
        if (bytes_received <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            Logger::get_logger()->error("Receive error: {}", strerror(errno));
            break;
        }
        
        try {
            std::string message(buffer_, bytes_received);
            Logger::get_logger()->debug("Received {} bytes from {}:{}", 
                         bytes_received, 
                         inet_ntoa(client_addr_.sin_addr), 
                         ntohs(client_addr_.sin_port));
            
            message_handler_(message, client_addr_);
        } catch (const std::exception& e) {
            Logger::get_logger()->error("Message handling error: {}", e.what());
        }
    }
}
void UdpServer::send(std::string_view message, const sockaddr_in& addr) {
    std::lock_guard<std::mutex> lock(send_mutex_);

    ssize_t sent_bytes = sendto(sockfd_, message.data(), message.size(), 0,
                                (struct sockaddr*)&addr, sizeof(addr));

    if (sent_bytes < 0) {
        Logger::get_logger()->error("UDP send failed: {}", strerror(errno));
    } else {
        Logger::get_logger()->debug("Sent {} bytes to {}:{}", sent_bytes,
                      inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }
}