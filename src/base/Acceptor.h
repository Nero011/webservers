#pragma once

#include <functional>

#include "Channel.h"
#include "Socket.h"
#include "noncopyable.h"
class EventLoop;
class InetAddress;

class Acceptor {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; };
    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};
