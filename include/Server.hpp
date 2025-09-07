#pragma once
#include <map>
#include <vector>
#include <string>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"
#include "Parser.hpp"

class Server {
public:
    Server(unsigned short port, const std::string& password);
    ~Server();
    void run();

    // handlers
    void handleCommand(Client& c, const Cmd& cmd);
    void cmdPASS(Client& c, const std::vector<std::string>& a);
    void cmdNICK(Client& c, const std::vector<std::string>& a);
    void cmdUSER(Client& c, const std::vector<std::string>& a);
    void cmdPING(Client& c, const std::vector<std::string>& a);
    void cmdQUIT(Client& c, const std::vector<std::string>& a);
    void cmdJOIN(Client& c, const std::vector<std::string>& a);
    void cmdPART(Client& c, const std::vector<std::string>& a);
    void cmdPRIVMSG(Client& c, const std::vector<std::string>& a);
    void cmdMODE (Client& c, const std::vector<std::string>& a);
    void cmdTOPIC(Client& c, const std::vector<std::string>& a);
    void cmdINVITE(Client& c, const std::vector<std::string>& a);
    void cmdKICK (Client& c, const std::vector<std::string>& a);

    void sendTo(Client& c, const std::string& msg);
    void markWrite(int fd);

private:
    int listen_fd;
    std::string password;
    std::vector<struct pollfd> pfds;
    std::map<int, Client> clients;
    std::map<std::string, Channel> channels;

    void initListen(unsigned short port);
    void setNonBlocking(int fd);
    void acceptNew();
    void handleRead(size_t idx);
    void handleWrite(size_t idx);
    void disconnect(size_t idx);
    void onLine(Client& c, const std::string& line);
    void tryRegister(Client& c);
    int  findFdByNick(const std::string& nick) const;
    void broadcastToChannel(const Channel& ch, int fromFd, const std::string& msg);
    void quitCleanup(Client& c, const std::string& reason);

};
