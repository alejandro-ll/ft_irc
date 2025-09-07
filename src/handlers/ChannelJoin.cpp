#include "Server.hpp"

void Server::cmdJOIN(Client& c, const std::vector<std::string>& a) {
    if (a.empty())
        return sendTo(c, ":server NOTICE " + (c.nick.empty()?"*":c.nick) + " :JOIN needs channel\r\n");

    std::string chan = a[0];
    if (chan.empty() || chan[0] != '#')
        return sendTo(c, ":server NOTICE " + c.nick + " :Bad channel name\r\n");

    std::string *keyOpt = 0;
    std::string keyTmp;
    if (a.size() >= 2) { keyTmp = a[1]; keyOpt = &keyTmp; }

    Channel &ch = channels[chan];               // crea si no existe
    if (ch.name.empty()) ch.name = chan;
    bool newlyCreated = ch.members.empty();

    if (!ch.canJoin(c.fd, keyOpt))
        return sendTo(c, ":server NOTICE " + c.nick + " :Cannot join channel\r\n");

    ch.addMember(c.fd);
    c.channels.insert(chan);
    if (newlyCreated) ch.addOp(c.fd);

    std::string joinMsg = ":" + (c.nick.empty()? "*": c.nick) + " JOIN " + chan + "\r\n";
    broadcastToChannel(ch, -1, joinMsg); // a todos
    //sendTo(c, joinMsg);                  // y al que entra (por si acaso)
}

void Server::cmdPART(Client& c, const std::vector<std::string>& a) {
    if (a.empty())
        return sendTo(c, ":server NOTICE " + (c.nick.empty()?"*":c.nick) + " :PART needs channel\r\n");

    std::string chan = a[0];
    std::map<std::string, Channel>::iterator it = channels.find(chan);
    if (it == channels.end())
        return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");

    Channel &ch = it->second;
    if (!ch.isMember(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not on that channel\r\n");

    std::string partMsg = ":" + c.nick + " PART " + chan + "\r\n";
    broadcastToChannel(ch, c.fd, partMsg);
    sendTo(c, partMsg);

    ch.removeMember(c.fd);
    c.channels.erase(chan);
    if (ch.members.empty()) channels.erase(it);
}
