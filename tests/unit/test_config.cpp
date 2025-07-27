// tests/unit/test_configs.cpp
#include "config/client_config.h"
#include "config/server_config.h"
#include <gtest/gtest.h>
#include <fstream>

class ConfigTest : public ::testing::Test {
protected:
    void createTestConfig(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        file << content;
        file.close();
    }
};

TEST_F(ConfigTest, ClientConfigValidation) {
    createTestConfig("test_client_config.json", R"({
        "server_ip": "127.0.0.1",
        "server_port": 8080,
        "log_file": "client.log",
        "log_level": "INFO",
        "request_timeout_ms": 1000,
        "max_retries": 3
    })");
    

    ASSERT_NO_THROW({
        ClientConfig config("test_client_config.json");
        EXPECT_EQ(config.get_server_ip(), "127.0.0.1");
        EXPECT_EQ(config.get_server_port(), 8080);
    });
    
    std::remove("test_client_config.json");
}

TEST_F(ConfigTest, InvalidClientConfigThrows) {
    createTestConfig("invalid_config.json", R"({
        "server_port": "not_a_number"
    })");

    EXPECT_THROW({
        ClientConfig config("invalid_config.json");
    }, std::exception);

    std::remove("invalid_config.json");
}

TEST_F(ConfigTest, ServerConfigValidation) {
    createTestConfig("test_server_config.json", R"({
        "udp_ip": "0.0.0.0",
        "udp_port": 5060,
        "http_port": 8080,
        "session_timeout_sec": 60,
        "cdr_file": "cdr.csv",
        "graceful_shutdown_rate": 10,
        "log_file": "server.log",
        "log_level": "INFO",
        "console_output": false,
        "blacklist": ["123456789012345"]
    })");
    
    
    ASSERT_NO_THROW({
        ServerConfig config("test_server_config.json");
        EXPECT_EQ(config.get_udp_port(), 5060);
        EXPECT_EQ(config.get_blacklist().size(), 1);
    });
    
    std::remove("test_server_config.json");
}