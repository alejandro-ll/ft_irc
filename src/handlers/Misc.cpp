#include "Server.hpp"

void Server::cmdPING(Client& c, const std::vector<std::string>& a){
    std::string token = a.empty()? "" : a.back();
    sendTo(c, ":server PONG :"+token+"\r\n");
}
// void Server::cmdQUIT(Client& c, const std::vector<std::string>& a){
//     (void)a;
//     for (size_t i=1;i<pfds.size();++i) if (pfds[i].fd==c.fd) { disconnect(i); break; }
// }

void Server::cmdQUIT(Client& c, const std::vector<std::string>& a) {
    const std::string reason = a.empty() ? "Client Quit" : a.back();
    quitCleanup(c, reason);
    c.closing = true;         
}

void Server::handleCommand(Client& c, const Cmd& cmd){
    if (cmd.verb == "PING") return cmdPING(c, cmd.args);
    if (cmd.verb == "QUIT") return cmdQUIT(c, cmd.args);

    if (!c.registered){
        if (cmd.verb == "PASS") return cmdPASS(c, cmd.args);
        if (cmd.verb == "NICK") return cmdNICK(c, cmd.args);
        if (cmd.verb == "USER") return cmdUSER(c, cmd.args);
        return sendTo(c, ":server NOTICE * :Register first (PASS/NICK/USER)\r\n");
    }

    if (cmd.verb == "JOIN")   return cmdJOIN(c, cmd.args);
    if (cmd.verb == "PART")   return cmdPART(c, cmd.args);
    if (cmd.verb == "PRIVMSG")return cmdPRIVMSG(c, cmd.args);
    if (cmd.verb == "MODE")  return cmdMODE(c, cmd.args);
    if (cmd.verb == "TOPIC") return cmdTOPIC(c, cmd.args);
    if (cmd.verb == "INVITE") return cmdINVITE(c, cmd.args);
    if (cmd.verb == "KICK") return cmdKICK(c, cmd.args);



    sendTo(c, ":server NOTICE " + (c.nick.empty()?"*":c.nick) + " :Unknown command\r\n");
}
