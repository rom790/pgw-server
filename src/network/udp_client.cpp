#include "network/udp_client.h"
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <chrono>
#include "utils/logger.h"

UdpClient::UdpClient(std::string_view server_ip, int server_port) {
    memset(&server_addr_, 0, sizeof(server_addr_));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip.data(), &server_addr_.sin_addr) <= 0) {
        Logger::get_logger()->error("Invalid server address: {}", server_ip);
        return;
    }
    
    if (!initialize()) {
        Logger::get_logger()->error("Failed to initialize UDP client");
    }
}

UdpClient::~UdpClient() {
    if (sockfd_ != -1) {
        close(sockfd_);
    }
}

bool UdpClient::initialize() {
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        Logger::get_logger()->error("Socket creation failed: {}", strerror(errno));
        return false;
    }
    
    // Таймаут на получение ответа
    std::chrono::seconds timeout{2};
    auto tv = std::chrono::duration_cast<std::chrono::microseconds>(timeout);
    timeval timeout_val{.tv_sec = tv.count() / 1000000, .tv_usec = tv.count() % 1000000};
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &timeout_val, sizeof(timeout_val));
    
    return true;
}

bool UdpClient::send(std::string_view message, std::string& response) {
    if (sockfd_ == -1) return false;
    

    ssize_t sent = sendto(sockfd_, message.data(), message.size(), 0,
                         (struct sockaddr*)&server_addr_, sizeof(server_addr_));
    if (sent < 0) {
        Logger::get_logger()->error("Send failed: {}", strerror(errno));
        return false;
    }
    Logger::get_logger()->info("Sent {} bytes to server", sent);
    
    // Получение ответа
    char buffer[1024];
    socklen_t addr_len = sizeof(server_addr_);
    ssize_t received = recvfrom(sockfd_, buffer, sizeof(buffer) - 1, 0,
                              (struct sockaddr*)&server_addr_, &addr_len);
    
    if (received <= 0) {
        Logger::get_logger()->error("Receive failed: {}", 
                     received == 0 ? "connection closed" : strerror(errno));
        return false;
    }
    
    buffer[received] = '\0';
    response = buffer;
    Logger::get_logger()->info("Received {} bytes from server: {}", received, response);
    
    return true;
}