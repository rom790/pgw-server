
#include <iostream>
#include "pgw/pgw_server.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [config_file]\n"
            << "Example:\n"
            << "  " << argv[0] << " config.json         # Interactive mode with custom config\n";
        return 1;
    }
    PgwServer server;
    try {
        server.init(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize server: " << e.what() << std::endl;
        return 1;
    }

    try {
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Server runtime error: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}