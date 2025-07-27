#include "cdr/cdr_manager.h"
#include <chrono>
#include <iomanip>
#include "utils/logger.h"

CdrManager::CdrManager(const std::string& filename)
    : file_(filename, std::ios::app),
      worker_(&CdrManager::process_queue, this)
{
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to open CDR file: " + filename);
    }

    Logger::get_logger()->info("CDR manager initialized with file: {}", filename);
}
CdrManager::~CdrManager() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
    flush();           
    file_.close();
}


void CdrManager::add_record(std::string_view imsi, std::string_view action) {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream record;
    record << std::put_time(std::localtime(&time), "%F %T") << ","
           << imsi << ","
           << action << "\n";
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(record.str());
    }
}

void CdrManager::flush() {
    std::queue<std::string> local_queue;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.swap(local_queue);
    }
    
    while (!local_queue.empty()) {
        file_ << local_queue.front();
        local_queue.pop();
    }
    
    file_.flush();
}

void CdrManager::process_queue() {
    while (running_) {
        // Запись каждые 100мс
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        flush();

        if (!running_) break;
    }
    
    Logger::get_logger()->debug("CDR worker thread stopped");
}