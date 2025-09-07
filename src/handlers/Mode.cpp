#include "Server.hpp"
#include <cstdlib>

static bool needsArg(char f){ return f=='k' || f=='o' || f=='l'; }

void Server::cmdMODE(Client& c, const std::vector<std::string>& a){
    if (a.empty()) return sendTo(c, ":server NOTICE " + c.nick + " :MODE needs a target\r\n");
    const std::string& target = a[0];
    if (target.empty() || target[0] != '#')
        return sendTo(c, ":server NOTICE " + c.nick + " :Only channel modes supported\r\n");

    std::map<std::string,Channel>::iterator it = channels.find(target);
    if (it == channels.end()) return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");
    Channel& ch = it->second;

    if (a.size()==1) {
        // Mostrar modos actuales (simple)
        std::string modes = "+";
        if (ch.inviteOnly)  modes += "i";
        if (ch.topicOpOnly) modes += "t";
        if (!ch.key.empty())modes += "k";
        if (ch.limit >= 0)  modes += "l";
        return sendTo(c, ":server NOTICE " + c.nick + " :" + target + " " + modes + "\r\n");
    }

    // Requiere ser operador del canal para cambiar modos
    if (!ch.isOp(c.fd)) return sendTo(c, ":server NOTICE " + c.nick + " :You're not channel operator\r\n");

    std::string modeStr = a[1];
    std::vector<std::string> args;
    for (size_t i=2;i<a.size();++i) args.push_back(a[i]);

    bool set = true;
    for (size_t i=0;i<modeStr.size();++i){
        char m = modeStr[i];
        if (m == '+'){ set = true;  continue; }
        if (m == '-'){ set = false; continue; }

        std::string arg;
        if (needsArg(m) && set){
            if (args.empty()) continue; // falta arg -> lo ignoramos
            arg = args.front(); args.erase(args.begin());
        }

        switch(m){
            case 'i': ch.setInviteOnly(set); break;
            case 't': ch.setTopicRestricted(set); break;
            case 'k':
                if (set) ch.setKey(arg); else ch.clearKey();
                break;
            case 'l':
                if (set) ch.setLimit(std::atoi(arg.c_str())); else ch.clearLimit();
                break;
            case 'o': {
                int fdTarget = findFdByNick(arg);
                if (fdTarget<0 || !ch.isMember(fdTarget)) break;
                if (set) ch.ops.insert(fdTarget); else ch.ops.erase(fdTarget);
            } break;
            default: /* ignorar */ break;
        }
    }

    // Anunciar (simple)
    std::string notif = ":" + c.nick + " MODE " + target + " " + modeStr;
    for (size_t i=0;i<args.size();++i) notif += " " + args[i]; // (solo si quieres reflejar)
    notif += "\r\n";
    broadcastToChannel(ch, -1, notif);
}
