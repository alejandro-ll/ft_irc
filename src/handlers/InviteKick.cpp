#include "Server.hpp"

void Server::cmdINVITE(Client& c, const std::vector<std::string>& a){
    if (a.size() < 2)
        return sendTo(c, ":server NOTICE " + c.nick + " :INVITE <nick> <#chan>\r\n");

    const std::string& nick = a[0];
    const std::string& chan = a[1];

    std::map<std::string,Channel>::iterator it = channels.find(chan);
    if (it == channels.end())
        return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");

    Channel& ch = it->second;

    if (!ch.isMember(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not on that channel\r\n");

    // Regla típica: si el canal es +i, solo los ops pueden invitar
    if (ch.inviteOnly && !ch.isOp(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not channel operator\r\n");

    int tfd = findFdByNick(nick);
    if (tfd < 0)
        return sendTo(c, ":server NOTICE " + c.nick + " :No such nick\r\n");

    ch.invite(tfd);

    // Notificar al invitado
    std::map<int,Client>::iterator itc = clients.find(tfd);
    if (itc != clients.end())
        sendTo(itc->second, ":" + c.nick + " INVITE " + nick + " :" + chan + "\r\n");

    // Confirmación al invitador
    sendTo(c, ":server NOTICE " + c.nick + " :Invited " + nick + " to " + chan + "\r\n");
}

void Server::cmdKICK(Client& c, const std::vector<std::string>& a){
    if (a.size() < 2)
        return sendTo(c, ":server NOTICE " + c.nick + " :KICK <#chan> <nick> [:reason]\r\n");

    const std::string& chan = a[0];
    const std::string& nick = a[1];
    std::string reason = (a.size() >= 3) ? a[2] : "Kicked";

    std::map<std::string,Channel>::iterator it = channels.find(chan);
    if (it == channels.end())
        return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");

    Channel& ch = it->second;

    if (!ch.isOp(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not channel operator\r\n");

    int tfd = findFdByNick(nick);
    if (tfd < 0 || !ch.isMember(tfd))
        return sendTo(c, ":server NOTICE " + c.nick + " :User not in channel\r\n");

    // Aviso a todos
    std::string msg = ":" + c.nick + " KICK " + chan + " " + nick + " :" + reason + "\r\n";
    broadcastToChannel(ch, -1, msg);

    // Echar al usuario del canal
    ch.removeMember(tfd);
    std::map<int,Client>::iterator itc = clients.find(tfd);
    if (itc != clients.end()) itc->second.channels.erase(chan);

    if (ch.members.empty()) channels.erase(it);
}
