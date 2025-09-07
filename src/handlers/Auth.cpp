#include "Server.hpp"

void Server::tryRegister(Client& c){
    if (!c.registered && c.passOk && !c.nick.empty() && !c.user.empty()){
        c.registered = true;
        sendTo(c, ":server 001 " + c.nick + " :Welcome to ft_irc\r\n");
    }
}
void Server::cmdPASS(Client& c, const std::vector<std::string>& a){
    if (a.empty()) return sendTo(c, ":server NOTICE * :PASS needs parameter\r\n");
    if (c.registered) return;
    c.passOk = (a[0] == password);
    if (!c.passOk) sendTo(c, ":server NOTICE * :Bad password\r\n");
    tryRegister(c);
}
void Server::cmdNICK(Client& c, const std::vector<std::string>& a){
    if (a.empty()) return sendTo(c, ":server NOTICE * :NICK needs parameter\r\n");
    // TODO: validar duplicados/formato
    c.nick = a[0];
    tryRegister(c);
}
void Server::cmdUSER(Client& c, const std::vector<std::string>& a){
    if (a.size() < 4) return sendTo(c, ":server NOTICE * :USER needs 4 params\r\n");
    c.user = a[0];
    c.realname = a.back();
    tryRegister(c);
}
