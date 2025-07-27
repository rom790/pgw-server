#include "network/udp_server.h"
#include "network/udp_client.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

class ServerClientIntegrationTest : public ::testing::Test {
protected:
    std::unique_ptr<UdpServer> server;
    std::thread server_thread;
    
    void SetUp() override {
        server = std::make_unique<UdpServer>("127.0.0.1", 5060, 
            [this](const std::string& msg, const sockaddr_in& addr) {

                this->server->send(msg, addr);
            });
        
        server_thread = std::thread([this]() {
            server->start();
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Время на запуск сервера
    }
    
    void TearDown() override {
        server->stop();
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }
};

TEST_F(ServerClientIntegrationTest, BasicCommunication) {
    UdpClient client("127.0.0.1", 5060);
    std::string response;
    
    EXPECT_TRUE(client.send("test message", response));
    EXPECT_EQ(response, "test message");
}

TEST_F(ServerClientIntegrationTest, LargeMessage) {
    UdpClient client("127.0.0.1", 5060);
    std::string response;
    std::string large_msg(1020, 'a'); // 1KB сообщение
    
    EXPECT_TRUE(client.send(large_msg, response));
    EXPECT_EQ(response, large_msg);
}