#pragma once

#include "noncopyable.h"

class InetAddress;

// 封装socketfd

class Socket : noncopyable {
private:
    const int sockfd_;

public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress &locoladdr);
    void listen();
    int connect(const InetAddress &addr);
    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
};
