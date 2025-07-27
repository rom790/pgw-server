#include "cdr/cdr_manager.h"
#include <gmock/gmock.h>
#include "session/session_manager.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using ::testing::_;

#include <iostream>

class MockCdrManager : public CdrManager {
public:
    MockCdrManager(const std::string& filename) : CdrManager(filename) {}
    
    MOCK_METHOD(void, add_record, (std::string_view imsi, std::string_view action), (override));
    MOCK_METHOD(void, flush, (), (override));
};


class SessionManagerTest : public ::testing::Test {
protected:
    std::shared_ptr<MockCdrManager> cdr_manager;
    std::unique_ptr<SessionManager> session_manager;
    
    void SetUp() override {
        cdr_manager = std::make_shared<MockCdrManager>("test_cdr.csv");
        session_manager = std::make_unique<SessionManager>(
            cdr_manager, 1, std::vector<std::string>{"123456789012345"}
        );
        
        // Ожидаем, что мок будет вызываться при создании сессии
        EXPECT_CALL(*cdr_manager, add_record(_, _)).Times(testing::AnyNumber());
    }
};



TEST_F(SessionManagerTest, CreateSessionWritesToCDR) {
    EXPECT_CALL(*cdr_manager, add_record("123456789012344", "created"))
        .Times(1);
        
    session_manager->create_session("123456789012344");
}