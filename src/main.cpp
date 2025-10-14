#include <iostream>
#include <cstdlib>
#include "Server.hpp"
#include <csignal>

// Variable global para controlar el shutdown
volatile sig_atomic_t g_shutdown = 0;

// Handler de señal
void signalHandler(int signal)
{
    (void)signal;
    g_shutdown = 1;
    std::cout << std::endl;
    std::cout << "\033[1;31m🔴 Received shutdown signal. Closing server...\033[0m" << std::endl;
}

void introServer()
{
    const std::string green = "\033[1;32m";
    const std::string yellow = "\033[1;33m";
    const std::string red = "\033[1;31m";
    const std::string cyan = "\033[1;36m";
    const std::string reset = "\033[0m";
    const std::string bold = "\033[1m";

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
    std::cout << red << bold;
    std::cout << "\n     ╔════════════════════════════════════╗\n";
    std::cout << "     ║     IRC COMMANDS   From HexChat    ║\n";
    std::cout << "     ╠════════════════════════════════════╣\n"
              << reset;
    std::cout << "     ║ Ctrl+C → Disconnect server         ║\n";
    std::cout << "     ║ " << green << "AUTHENTICATION" << reset << "                     ║\n";
    std::cout << "     ║   /PASS <password>                 ║\n";
    std::cout << "     ║   /NICK <nickname>                 ║\n";
    std::cout << "     ║   /USER <user> <0> <0> <name>      ║\n";
    std::cout << "     ║ " << green << "CHANNELS" << reset << "                           ║\n";
    std::cout << "     ║   /JOIN <channel> [:key]           ║\n";
    std::cout << "     ║   /PART <channel>                  ║\n";
    std::cout << "     ║   /TOPIC <channel> [:topic]        ║\n";
    std::cout << "     ║ " << green << "OPERATORS" << reset << "                          ║\n";
    std::cout << "     ║   /KICK <channel> <user>           ║\n";
    std::cout << "     ║   /INVITE <user> <channel>         ║\n";
    std::cout << "     ║   /MODE <channel> <mode>           ║\n";
    std::cout << "     ║ " << yellow << "CHANNEL MODES" << reset << "                      ║\n";
    std::cout << "     ║   +i/-i → Invite-only              ║\n";
    std::cout << "     ║   +t/-t → Topic restriction        ║\n";
    std::cout << "     ║   +k/-k <key> → Password           ║\n";
    std::cout << "     ║   +o/-o <user> → Operator          ║\n";
    std::cout << "     ║   +l/-l <limit> → User limit       ║\n";
    std::cout << "     ║ " << green << "MESSAGING" << reset << "                          ║\n";
    std::cout << "     ║   /PRIVMSG <target> <msg>          ║\n";
    std::cout << "     ║   /QUIT [:message]                 ║\n";
    std::cout << "     ║ " << cyan << "NETCAT" << reset << "                             ║\n";
    std::cout << "     ║   nc -C 127.0.0.1 <PORT>           ║\n";
    std::cout << "     ╚════════════════════════════════════╝\n"
              << reset;
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
