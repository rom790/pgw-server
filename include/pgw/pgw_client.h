#pragma once
#include <memory>
#include <string>
#include "config/client_config.h"
#include "network/udp_client.h"

class PgwClient {
public:
    explicit PgwClient() = default;
    void init(const std::string& config_file);

    std::string send_imsi(const std::string& imsi);

    void interactive_mode();
    
    // Запрещаем копирование и присваивание
    PgwClient(const PgwClient&) = delete;
    PgwClient& operator=(const PgwClient&) = delete;

private:
    std::unique_ptr<ClientConfig> config_;      // Конфигурация клиента
    std::unique_ptr<UdpClient> udp_client_;     // UDP транспорт

    bool is_ready() const {
        return udp_client_ && udp_client_->is_initialized();
    }
};