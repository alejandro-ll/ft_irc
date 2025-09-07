#include "Server.hpp"
#include <stdexcept>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Server::Server(unsigned short port, const std::string& pwd)
: listen_fd(-1), password(pwd) {
    initListen(port);
    struct pollfd p; p.fd = listen_fd; p.events = POLLIN; p.revents = 0;
    pfds.push_back(p);
}
Server::~Server(){ if (listen_fd>=0) ::close(listen_fd); }

void Server::setNonBlocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) flags = 0;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
void Server::initListen(unsigned short port){
    listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) throw std::runtime_error("socket failed");
    int yes=1; ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    sockaddr_in addr; std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = htonl(INADDR_ANY); addr.sin_port = htons(port);
    if (::bind(listen_fd, (sockaddr*)&addr, sizeof(addr))<0) throw std::runtime_error("bind failed");
    if (::listen(listen_fd, 128)<0) throw std::runtime_error("listen failed");
    setNonBlocking(listen_fd);
}

void Server::run(){
    for(;;){
        for (size_t i=1;i<pfds.size();++i){
            int fd = pfds[i].fd;
            pfds[i].events = clients[fd].sendBuf.empty()? POLLIN : (POLLIN|POLLOUT);
        }
        int n = ::poll(&pfds[0], pfds.size(), -1);
        if (n < 0){ if (errno==EINTR) continue; throw std::runtime_error("poll failed"); }

        if (pfds[0].revents & POLLIN) acceptNew();

        for (size_t i=1;i<pfds.size();++i){
            short ev = pfds[i].revents;
            if (ev & (POLLERR|POLLHUP|POLLNVAL)) { disconnect(i--); continue; }
            if (ev & POLLIN)  handleRead(i);
            if (i<pfds.size() && (pfds[i].revents & POLLOUT)) handleWrite(i);
        }
    }
}

void Server::acceptNew(){
    for(;;){
        int cfd = ::accept(listen_fd, NULL, NULL);
        if (cfd < 0){
            if (errno==EAGAIN || errno==EWOULDBLOCK) break;
            std::perror("accept"); break;
        }
        setNonBlocking(cfd);
        clients.insert(std::make_pair(cfd, Client(cfd)));
        struct pollfd p; p.fd=cfd; p.events=POLLIN; p.revents=0;
        pfds.push_back(p);
        std::fprintf(stderr, "Nuevo cliente (fd=%d)\n", cfd);
    }
}

void Server::handleRead(size_t idx){
    int fd = pfds[idx].fd;
    Client &c = clients[fd];
    char buf[4096];
    for(;;){
        ssize_t r = ::recv(fd, buf, sizeof(buf), 0);
        if (r > 0){
            c.recvBuf.append(buf, r);
            std::string::size_type pos;
            while ((pos = c.recvBuf.find("\r\n")) != std::string::npos) {
                std::string line = c.recvBuf.substr(0, pos);
                c.recvBuf.erase(0, pos + 2);
                onLine(c, line);
            }
        } else if (r == 0){
            disconnect(idx); return;
        } else {
            if (errno==EAGAIN || errno==EWOULDBLOCK) break;
            disconnect(idx); return;
        }
    }
}
void Server::handleWrite(size_t idx){
    int fd = pfds[idx].fd;
    Client &c = clients[fd];
    while (!c.sendBuf.empty()){
        ssize_t n = ::send(fd, c.sendBuf.data(), c.sendBuf.size(), 0);
        if (n > 0) c.sendBuf.erase(0, n);
        else {
            if (errno==EAGAIN || errno==EWOULDBLOCK) return;
            disconnect(idx); return;
        }
    }
    pfds[idx].events &= ~POLLOUT;
}
void Server::disconnect(size_t idx){
    int fd = pfds[idx].fd;
    // TODO: sacar de canales si aplica
    ::close(fd);
    clients.erase(fd);
    pfds[idx] = pfds.back(); pfds.pop_back();
}
void Server::sendTo(Client& c, const std::string& msg){ c.sendBuf += msg; markWrite(c.fd); }
void Server::markWrite(int fd){ for(size_t i=1;i<pfds.size();++i) if (pfds[i].fd==fd){ pfds[i].events |= POLLOUT; break; } }
void Server::onLine(Client& c, const std::string& line){ Cmd cmd = parseIrcLine(line); handleCommand(c, cmd); }

int Server::findFdByNick(const std::string& nick) const {
    for (std::map<int, Client>::const_iterator it=clients.begin(); it!=clients.end(); ++it)
        if (it->second.nick == nick) return it->first;
    return -1;
}
void Server::broadcastToChannel(const Channel& ch, int fromFd, const std::string& msg){
    for (std::set<int>::const_iterator it=ch.members.begin(); it!=ch.members.end(); ++it){
        if (*it == fromFd) continue;
        std::map<int, Client>::iterator ci = clients.find(*it);
        if (ci != clients.end()) sendTo(ci->second, msg);
    }
}

void Server::quitCleanup(Client& c, const std::string& reason){
    const std::string quitMsg = ":" + (c.nick.empty() ? "*" : c.nick) + " QUIT :" + reason + "\r\n";
    std::vector<std::string> chans(c.channels.begin(), c.channels.end());
    for (size_t i = 0; i < chans.size(); ++i) {
        std::map<std::string, Channel>::iterator it = channels.find(chans[i]);
        if (it == channels.end()) continue;
        Channel &ch = it->second;
        broadcastToChannel(ch, c.fd, quitMsg);
        ch.removeMember(c.fd);
        c.channels.erase(chans[i]);
        if (ch.members.empty()) channels.erase(it);
        else if (ch.ops.empty()) {
            int promote = *ch.members.begin();
            ch.addOp(promote);
        }
    }
}
