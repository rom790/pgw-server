#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <shared_mutex>
#include <fstream>
#include <deque>
#include <string_view>
#include "utils/logger.h"
#include "cdr/cdr_manager.h"

class SessionManager {
public:
    SessionManager(
        std::shared_ptr<CdrManager> cdr_manager,
        int session_timeout_sec,
        const std::vector<std::string>& blacklist
    );
    bool create_session(std::string_view imsi);
    bool session_exists(std::string_view imsi) const;
    void cleanup_expired_sessions();
    void graceful_shutdown(int sessions_per_sec);
    bool is_blacklisted(const std::string& imsi) const;

private:
    struct Session {
        std::chrono::steady_clock::time_point expires_at;
    };

    mutable std::shared_mutex sessions_mutex_;
    std::unordered_map<std::string, Session> sessions_;
    std::deque<std::pair<std::chrono::steady_clock::time_point, std::string>> expiry_queue_;
    std::unordered_set<std::string> blacklist_;

    std::shared_ptr<CdrManager> cdr_manager_;
    std::mutex cdr_mutex_;
    const int session_timeout_sec_;

    void write_cdr(std::string_view imsi, std::string_view action) const;
    bool validate_imsi(std::string_view imsi) const;
};