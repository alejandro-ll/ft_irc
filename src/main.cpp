#include <iostream>
#include <cstdlib>
#include "Server.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
        return 1;
    }
    unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));
    std::string password = argv[2];
    try {
        Server srv(port, password);
        srv.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
