#include "Server.hpp"

void Server::cmdTOPIC(Client& c, const std::vector<std::string>& a){
    if (a.empty()) return sendTo(c, ":server NOTICE " + c.nick + " :TOPIC needs #channel\r\n");
    const std::string& chan = a[0];
    std::map<std::string,Channel>::iterator it = channels.find(chan);
    if (it == channels.end()) return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");

    Channel& ch = it->second;
    // Mostrar topic si no hay texto
    if (a.size() == 1) {
        std::string t = ch.topic.empty()? "No topic is set" : ch.topic;
        return sendTo(c, ":server NOTICE " + c.nick + " :" + chan + " : " + t + "\r\n");
    }

    // Cambiar topic: validar permisos
    if (ch.topicOpOnly && !ch.isOp(c.fd))
        return sendTo(c, ":server NOTICE " + c.nick + " :You're not channel operator\r\n");

    ch.topic = a[1]; // llega como trailing ya
    std::string msg = ":" + c.nick + " TOPIC " + chan + " :" + ch.topic + "\r\n";
    broadcastToChannel(ch, -1, msg);
}
