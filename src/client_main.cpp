#include <iostream>
#include "pgw/pgw_client.h"


int main(int argc, char* argv[]) {

    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " [config_file] [IMSI]\n"
                  << "Examples:\n"
                  << "  " << argv[0] << " config.json         # Interactive mode with config\n"
                  << "  " << argv[0] << " config.json 123456  # Single request mode\n";
        return 1;
    }

    try {
        PgwClient client;
        try {   
            client.init(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize client: " << e.what() << std::endl;
            return 1;
        }

        // Режим работы
        if (argc == 3) {
            // Режим однократного запроса
            std::string response = client.send_imsi(argv[2]);
            std::cout << response << std::endl;
        } else {
            // Интерактивный режим
            client.interactive_mode();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}