#include <iostream>
#include <cstdlib>
#include "Server.hpp"

#include <iostream>

/**
 * @brief Prints a green ASCII art banner for the IRC server
 */
#include <iostream>

#include <iostream>

void introServer()
{
    const std::string green = "\033[1;32m";
    const std::string reset = "\033[0m";

    std::cout << green << std::endl;

    std::cout << "░▀█▀░█▀▄░█▀▀░░░█▀▀░█▀▀░█▀▄░█░█░█▀▀░█▀▄" << std::endl;
    std::cout << "░░█░░█▀▄░█░░░░░▀▀█░█▀▀░█▀▄░▀▄▀░█▀▀░█▀▄" << std::endl;
    std::cout << "░▀▀▀░▀░▀░▀▀▀░░░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░▀" << std::endl;

    std::cout << reset << std::endl;
}

/**
 * @brief Main entry point for IRC server application
 * @param argv Argument vector [program, port, password]
 * @details Initializes and runs IRC server on specified port with password authentication
 */
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
        return 1;
    }
    unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));
    std::string password = argv[2];
    try
    {
        introServer();
        Server srv(port, password);
        srv.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
