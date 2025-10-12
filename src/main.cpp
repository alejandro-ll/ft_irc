#include <iostream>
#include <cstdlib>
#include "Server.hpp"
#include <iostream>

#include <cstdlib>
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
    std::cout << cyan << "    Connect a client using netcat (nc):  nc -C 127.0.0.1 <PORT>" << reset << std::endl;
    std::cout << std::endl;

    std::cout << green << "    Authentication Commands" << reset << std::endl;
    std::cout << "    - PASS <password>  ->  Verify server password" << std::endl;
    std::cout << "    - NICK <nickname>  ->  Set your nickname" << std::endl;
    std::cout << "    - USER <user> <unused> <unused> <realname> ->  Set user information" << std::endl;

    std::cout << green << "    Channel Commands" << reset << std::endl;
    std::cout << "    - JOIN <channel>   ->  Join a channel" << std::endl;
    std::cout << "    - PART <channel>   ->  Leave a channel" << std::endl;
    std::cout << "    - TOPIC <channel> : [topic] ->  Manage channel topic" << std::endl;

    std::cout << green << "    Operator Commands" << reset << std::endl;
    std::cout << "    - MODE <channel> <modes> [params] -> Set channel modes" << std::endl;
    std::cout << "    - KICK <channel> <user> : [reason] -> Kick users from channels" << std::endl;
    std::cout << "    - INVITE <user> <channel> ->  Invite users to channels" << std::endl;

    std::cout << yellow << "    Channel Modes (MODE command)" << reset << std::endl;
    std::cout << "    - +i / -i : Set/remove Invite-only channel" << std::endl;
    std::cout << "    - +t / -t : Restrict TOPIC command to channel operators" << std::endl;
    std::cout << "    - +k / -k <key> : Set/remove channel password" << std::endl;
    std::cout << "    - +o / -o <user> : Give/take channel operator privilege" << std::endl;
    std::cout << "    - +l / -l <limit> : Set/remove user limit" << std::endl;

    std::cout << green << "    Communication Commands" << reset << std::endl;
    std::cout << "    - PRIVMSG <target> : <message> ->  Send private or channel messages" << std::endl;
    std::cout << "    - NOTICE <target> : <message>  ->  Send notice messages" << std::endl;
    std::cout << "    - PING <server>    ->  Connectivity check" << std::endl;
    std::cout << "    - QUIT : [message]   ->  Disconnect from server" << std::endl;
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
