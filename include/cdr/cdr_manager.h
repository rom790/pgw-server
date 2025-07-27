#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <string_view>

class CdrManager {
public:
    CdrManager(const std::string& filename);
    ~CdrManager() noexcept;
    
    virtual void add_record(std::string_view imsi, std::string_view action);
    virtual void flush();

private:
    std::ofstream file_;
    std::mutex mutex_;
    std::queue<std::string> queue_;
    std::atomic<bool> running_{true};
    std::thread worker_;
    
    void process_queue();
};