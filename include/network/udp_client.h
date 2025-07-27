#pragma once
#include <string>
#include <netinet/in.h>
#include <string_view>
#include <arpa/inet.h>

class UdpClient {
public:
    UdpClient(std::string_view server_ip, int server_port);
    ~UdpClient();
    
    bool send(std::string_view message, std::string& response);
    bool is_initialized() const { return sockfd_ != -1; }

private:
    int sockfd_ = -1;
    struct sockaddr_in server_addr_;
    
    bool initialize();
};