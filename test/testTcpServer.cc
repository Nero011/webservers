#include <functional>
#include <iostream>
#include <string>

#include "mymuduo/TcpServer.h"

// g++ -o testserver test/testTcpServer.cc -lmymuduo -lpthread -g

class Server {
private:
    TcpServer server_;
    EventLoop* loop_;

    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            std::cout << "Connection up";
            std::cout << conn->peerAddress().toIp() << " " << conn->peerAddress().toPort() << std::endl;
        } else {
            std::cout << "Connection down" << std::endl;
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receviceTime) {
        std::string s = buf->retrieveAllAsString();
        conn->send(s);
        conn->shutdown();
    }

public:
    Server(EventLoop* loop, const std::string& name, const InetAddress& local)
        : loop_(loop), server_(loop, local, name, TcpServer::kReuseProt) {
        server_.setConnectionCallback(std::bind(&Server::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
            std::bind(&Server::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
    ~Server() {}

    void start() {
        server_.start();
        loop_->loop();
    }
};

int main() {
    EventLoop loop;
    InetAddress local(8008);
    Server echo(&loop, "echo", local);

    echo.start();

    return 0;
}
