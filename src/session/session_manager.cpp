#include "session/session_manager.h"
#include <iomanip>
#include "utils/logger.h"
#include <iostream>


SessionManager::SessionManager(std::shared_ptr<CdrManager> cdr_manager,
                               int session_timeout_sec,
                               const std::vector<std::string>& blacklist)
    : cdr_manager_(std::move(cdr_manager)),
      session_timeout_sec_(session_timeout_sec) {
    
    for (const auto& imsi : blacklist) {
        if (validate_imsi(imsi)) {
            blacklist_.insert(imsi);
        }
    }
}


bool SessionManager::create_session(std::string_view imsi) {
    if (!validate_imsi(imsi)) return false;
    
    if (blacklist_.find(std::string(imsi)) != blacklist_.end()) {
        write_cdr(imsi, "rejected_blacklist");
        return false;
    }

    auto expires_at = std::chrono::steady_clock::now() + 
                      std::chrono::seconds(session_timeout_sec_);

    std::unique_lock lock(sessions_mutex_);
    
    auto [it, inserted] = sessions_.try_emplace(std::string(imsi), Session{expires_at});
    
    if (inserted) {
        write_cdr(imsi, "created");
    } else {
        it->second.expires_at = expires_at;
        write_cdr(imsi, "prolonged");
    }

    expiry_queue_.emplace_back(expires_at, imsi);
    
    return true;
}



bool SessionManager::session_exists(std::string_view imsi) const {
    std::shared_lock lock(sessions_mutex_);
    return sessions_.contains(std::string(imsi));
}

void SessionManager::cleanup_expired_sessions() {
    auto now = std::chrono::steady_clock::now();
    std::unique_lock lock(sessions_mutex_);

    while (!expiry_queue_.empty()) {
        const auto& [expires_at, imsi] = expiry_queue_.front();

        if (expires_at > now) {
            break;
        }

        expiry_queue_.pop_front();

        auto it = sessions_.find(imsi);
        if (it != sessions_.end()) {
            if (it->second.expires_at <= now) {
                write_cdr(imsi, "expired");
                sessions_.erase(it);
            }
        }
    }
}


void SessionManager::graceful_shutdown(int sessions_per_sec) {
    const auto delay = sessions_per_sec > 0 ? 
        std::chrono::milliseconds(1000) : 
        std::chrono::milliseconds(0);

    while (!sessions_.empty()) {
        std::vector<std::string> to_remove;

        {
            std::unique_lock lock(sessions_mutex_);

            int count = std::min(sessions_per_sec, static_cast<int>(sessions_.size()));
            auto it = sessions_.begin();
            for (int i = 0; i < count && it != sessions_.end(); ++i, ++it) {
                to_remove.push_back(it->first);
            }

            for (const auto& imsi : to_remove) {
                sessions_.erase(imsi);
            }
        }

        for (const auto& imsi : to_remove) {
            write_cdr(imsi, "graceful_removal");
        }

        if (!to_remove.empty() && delay.count() > 0) {
            std::this_thread::sleep_for(delay);
        }
    }
}
bool SessionManager::is_blacklisted(const std::string& imsi) const {
    return blacklist_.contains(imsi);
}

void SessionManager::write_cdr(std::string_view imsi, std::string_view action) const{
    if (cdr_manager_) {
        cdr_manager_->add_record(imsi, action);
    } else {
        Logger::get_logger()->warn("CDR manager is null; skipping record for {} ({})", imsi, action);
    }
}

bool SessionManager::validate_imsi(std::string_view imsi) const {
    return !imsi.empty() && imsi.size() <= 15 &&
           std::all_of(imsi.begin(), imsi.end(), ::isdigit);
}
