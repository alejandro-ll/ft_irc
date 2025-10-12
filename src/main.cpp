#include <iostream>
#include <cstdlib>
#include "Server.hpp"
#include <iostream>

#include <cstdlib>
#include <csignal>

/**
 * @brief Prints a green ASCII art banner for the IRC server
 */

// Variable global para controlar el shutdown
volatile sig_atomic_t g_shutdown = 0;

// Handler de señal
void signalHandler(int signal)
{
    (void)signal;
    g_shutdown = 1;
    std::cout << std::endl;
    std::cout << "🔴 Received shutdown signal. Closing server..." << std::endl;
}

void introServer()
{
    const std::string green = "\033[1;32m";
    const std::string reset = "\033[0m";
    const std::string red = "\033[1;31m"; // rojo negrita

    std::cout << green << std::endl;

    std::cout << "               ██╗██████╗  ██████╗" << std::endl;
    std::cout << "               ██║██╔══██╗██╔════╝" << std::endl;
    std::cout << "               ██║██████╔╝██║     " << std::endl;
    std::cout << "               ██║██╔══██╗██║     " << std::endl;
    std::cout << "               ██║██║  ██║╚██████╗" << std::endl;
    std::cout << "               ╚═╝╚═╝  ╚═╝ ╚═════╝" << std::endl;
    std::cout << std::endl;
    std::cout << "  ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗ " << std::endl;
    std::cout << "  ██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗" << std::endl;
    std::cout << "  ███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝" << std::endl;
    std::cout << "  ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗" << std::endl;
    std::cout << "  ███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║" << std::endl;
    std::cout << "  ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝" << std::endl;
    std::cout << red << std::endl;
    std::cout << "       To disconnect the server, press Ctrl+C" << std::endl;

    std::cout << reset << std::endl;
    std::cout << green << "    Connect a client using netcat (nc):  nc -C 127.0.0.1 6667" << reset << std::endl;
    std::cout << std::endl;
    std::cout << green << "    Authentication Commands" << reset << std::endl;
    std::cout << "    - PASS: Verify server password" << std::endl;
    std::cout << "    - NICK: Set your nickname" << std::endl;
    std::cout << "    - USER: Set user information" << std::endl;

    std::cout << green << "    Channel Commands" << reset << std::endl;
    std::cout << "    - JOIN: Join a channel" << std::endl;
    std::cout << "    - PART: Leave a channel" << std::endl;
    std::cout << "    - TOPIC: Manage channel topic" << std::endl;

    std::cout << green << "    Operator Commands" << reset << std::endl;
    std::cout << "    - MODE: Set channel/user modes" << std::endl;
    std::cout << "    - KICK: Kick users from channels" << std::endl;
    std::cout << "    - INVITE: Invite users to channels" << std::endl;

    std::cout << green << "    Communication Commands" << reset << std::endl;
    std::cout << "    - PRIVMSG: Send private or channel messages" << std::endl;
    std::cout << "    - PING: Connectivity check" << std::endl;
    std::cout << "    - QUIT: Disconnect from server" << std::endl;
    std::cout << std::endl;
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

    std::signal(SIGINT, signalHandler);  /* Ctrl+C */
    std::signal(SIGTERM, signalHandler); /* kill command */

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
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
