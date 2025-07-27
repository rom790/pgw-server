#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <mutex>
#include <numeric>
#include <string>
#include "pgw/pgw_client.h"

std::mutex cout_mutex;
std::mutex stats_mutex;

struct ClientStats {
    double init_time_ms = 0;
    double send_time_ms = 0;
    bool success = false;
};

std::vector<ClientStats> global_stats;
bool verbose_output = true;
std::string config_path;

void test_client(const std::string& imsi, int thread_id) {
    ClientStats stats;
    
    try {
        // Инициализация клиента
        auto start_init = std::chrono::high_resolution_clock::now();
        PgwClient client;
        client.init(config_path);

        auto init_time = std::chrono::high_resolution_clock::now() - start_init;
        stats.init_time_ms = std::chrono::duration<double, std::milli>(init_time).count();

        // Отправка IMSI 
        auto start_send = std::chrono::high_resolution_clock::now();
        std::string response = client.send_imsi(imsi);

        auto send_time = std::chrono::high_resolution_clock::now() - start_send;
        stats.send_time_ms = std::chrono::duration<double, std::milli>(send_time).count();
        stats.success = true;

        // Вывод результатов
        if (verbose_output) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Thread " << thread_id << " | IMSI: " << imsi 
                      << " | Init: " << stats.init_time_ms << " ms"
                      << " | Send: " << stats.send_time_ms << " ms"
                      << " | Response: " << response << "\n";
        }
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Error in thread " << thread_id << ": " << e.what() << "\n";
    }

    std::lock_guard<std::mutex> lock(stats_mutex);
    global_stats.push_back(stats);
}

void print_final_stats() {
    if (global_stats.empty()) {
        std::cout << "No statistics collected\n";
        return;
    }

    std::vector<ClientStats> successful_stats;
    for (const auto& stat : global_stats) {
        if (stat.success) {
            successful_stats.push_back(stat);
        }
    }

    if (successful_stats.empty()) {
        std::cout << "No successful requests\n";
        return;
    }
    // Среднее время для инициализации клиента и для отправки сообщения
    double avg_init = std::accumulate(
        successful_stats.begin(), successful_stats.end(), 0.0,
        [](double sum, const ClientStats& s) { return sum + s.init_time_ms; }
    ) / successful_stats.size();

    double avg_send = std::accumulate(
        successful_stats.begin(), successful_stats.end(), 0.0,
        [](double sum, const ClientStats& s) { return sum + s.send_time_ms; }
    ) / successful_stats.size();

    double avg_total = avg_init + avg_send;

    std::cout << "\nStatistics:\n";
    std::cout << "Total requests: " << global_stats.size() << "\n";
    std::cout << "Successful requests: " << successful_stats.size() << "\n";
    std::cout << "Average init time: " << avg_init << " ms\n";
    std::cout << "Average send time: " << avg_send << " ms\n";
    std::cout << "Average total time per request: " << avg_total << " ms\n";
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <config_path> [num_clients] [verbose]\n"
              << "Arguments:\n"
              << "  config_path    Path to client config file (required)\n"
              << "  num_clients    Number of clients to simulate (default: 100)\n"
              << "  verbose        Show detailed output (0/1, default: 1)\n\n"
              << "Examples:\n"
              << "  " << program_name << " config.json              # Default: 100 clients, verbose output\n"
              << "  " << program_name << " config.json 500 0        # 500 clients, silent mode\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: Config file path is required\n";
        print_usage(argv[0]);
        return 1;
    }

    config_path = argv[1];
    
    int num_clients = 100;
    if (argc > 2) {
        try {
            num_clients = std::stoi(argv[2]);
            if (num_clients <= 0) {
                throw std::invalid_argument("Number of clients must be positive");
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid number of clients: " << e.what() << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (argc > 3) {
        try {
            int verbose = std::stoi(argv[3]);
            verbose_output = (verbose != 0);
        } catch (const std::exception& e) {
            std::cerr << "Invalid verbose flag: " << e.what() << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (argc > 4) {
        print_usage(argv[0]);
        return 1;
    }

    const int num_threads = std::thread::hardware_concurrency();

    if (verbose_output) {
        std::cout << "Starting load test with " << num_clients 
                  << " clients using " << num_threads << " threads...\n";
        std::cout << "Using config file: " << config_path << "\n";
    }

    auto test_start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    // Запуск клиентов группами по количеству доступных ядер
    for (int i = 0; i < num_clients; i += num_threads) {
        for (int j = 0; j < num_threads && (i + j) < num_clients; ++j) {
            std::string imsi = "123456789" + std::to_string(i + j);
            threads.emplace_back(test_client, imsi, i + j);
        }

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }
        threads.clear();
    }

    auto total_dur = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - test_start
    );

    print_final_stats();
    std::cout << "\nTotal test duration: " << total_dur.count() << " ms\n";
    std::cout << "Requests per second: " 
              << (global_stats.size() / (total_dur.count() / 1000.0)) 
              << " req/s\n";

    return 0;
}

