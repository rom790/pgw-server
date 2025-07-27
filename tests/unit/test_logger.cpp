// tests/unit/test_logger.cpp
#include "utils/logger.h"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

class LoggerTest : public ::testing::Test {
protected:
    const std::string test_log_file = "test_log.log";
    
    void SetUp() override {
        Logger::init(test_log_file, "test_logger", "TRACE");
    }
    
    void TearDown() override {
        spdlog::drop_all();
        std::remove(test_log_file.c_str());
    }
};

TEST_F(LoggerTest, LogLevels) {
    Logger::debug("Test debug message");
    Logger::info("Test info message");
    Logger::warn("Test warning message");
    Logger::error("Test error message");
    
    std::ifstream log_file(test_log_file);
    std::string line;
    int line_count = 0;
    
    while (std::getline(log_file, line)) {
        line_count++;
    }
    
    EXPECT_GE(line_count, 4);
}

TEST_F(LoggerTest, FileCreation) {
    EXPECT_TRUE(std::filesystem::exists(test_log_file));
}