#include <filesystem>
#include <iostream>
#include <stdexcept>
#include "utils/logger.h"
#include "utils/bcd_converter.h"
#include "pgw/pgw_client.h"

void PgwClient::init(const std::string& config_file) {

    if (config_file.empty()) {
        throw std::runtime_error("Config file path cannot be empty");
    }

    if (!std::filesystem::exists(config_file)) {
        throw std::runtime_error("Config file not found: " + config_file);
    }
    if (!std::filesystem::is_regular_file(config_file)) {
        throw std::runtime_error("Config file is not a regular file: " + config_file);
    }

    config_ = std::make_unique<ClientConfig>(config_file);

    if (!config_->get_log_file().empty()) {
        Logger::init(config_->get_log_file(),"client_logger", config_->get_log_level(), config_->get_console_output());
    }

    udp_client_ = std::make_unique<UdpClient>(
        config_->get_server_ip(),
        config_->get_server_port()
    );

    Logger::get_logger()->info("PGW Client initialized for server {}:{}",
                 config_->get_server_ip(),
                 config_->get_server_port());

}


std::string PgwClient::send_imsi(const std::string& imsi) {
    auto start_f = std::chrono::high_resolution_clock::now();
    if (!udp_client_ || !udp_client_->is_initialized()) {
        Logger::get_logger()->error("UDP client not initialized");
        return "client_error";
    }

    if (!BCDConverter::validate_imsi(imsi)) {
        Logger::get_logger()->error("Invalid IMSI format: {}", imsi);
        return "invalid_imsi";
    }

    Logger::get_logger()->debug("Sending IMSI: {}", imsi);

    std::vector<uint8_t> bcd_data;
    try {
        bcd_data = BCDConverter::imsi_to_bcd(imsi);
    } catch (const std::exception& e) {
        Logger::get_logger()->error("BCD conversion failed: {}", e.what());
        return "bcd_error";
    }


    auto start_th = std::chrono::high_resolution_clock::now();
    std::string response;
    if (!udp_client_->send(
            std::string(bcd_data.begin(), bcd_data.end()), 
            response)) {
        Logger::get_logger()->error("Failed to send/receive data");
        return "network_error";
    }

    Logger::get_logger()->info("Received server response: {}", response);
    return response;
}

void PgwClient::interactive_mode() {
    if (!udp_client_ || !udp_client_->is_initialized()) {
        std::cerr << "Client not initialized" << std::endl;
        return;
    }

    std::cout << "PGW Client Interactive Mode" << std::endl;
    std::cout << "Enter IMSI (10 - 15 digits) or 'q' to quit" << std::endl;

    while (true) {
        std::string input;
        std::cout << "IMSI> ";
        std::getline(std::cin, input);

        if (input == "q" || input == "quit") {
            break;
        }
        std::string response = send_imsi(input);
        std::cout << "Response: " << response << std::endl;
    }
}

