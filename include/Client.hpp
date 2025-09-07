#pragma once
#include <string>
#include <set>

struct Client {
    int fd;
    std::string recvBuf, sendBuf;
    std::string nick, user, realname;
    bool passOk, registered;
    bool closing;
    std::set<std::string> channels;

    Client(): fd(-1), passOk(false), registered(false), closing(false) {}
    explicit Client(int f): fd(f), passOk(false), registered(false), closing(false) {}
};
