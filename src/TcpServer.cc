#include "TcpServer.h"

#include <string.h>

#include <functional>

#include "Logger.h"
#include "TcpConnection.h"

EventLoop *CheckLoopNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        // LOG FATAL
        exit(0);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option)
    : loop_(CheckLoopNotNull(loop))
    , ipPort_(listenAddr.toIpPort())
    , name_(nameArg)
    , acceptor_(new Acceptor(loop, listenAddr, option == kReuseProt))
    , threadPool_(new EventLoopThreadPool(loop, name_))
    , started_(false)
    , nextConnId_(1) {
    acceptor_->setNewConnectCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}
TcpServer::~TcpServer() {
    for (auto &item : connections_) {
        // 这个局部shared_ptr，离开当前for作用域后，可以自动释放new的TcpConnnection
        TcpConnectionPtr conn(item.second);
        item.second.reset();

        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connnectionDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) { threadPool_->setThreadNums(numThreads); }

void TcpServer::start() {
    if (started_++ == 0) {                                                // 防止一个TcpServer对象被start多次
        threadPool_->start(threadInitCallback_);                          // 启动底层的线程池
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));  // 手动启动baseloop，调用acceptor的listen
    }
}

// 有一个新的客户端连接，回执行这个回调
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    EventLoop *ioloop = threadPool_->getNextLoop();  // 轮询算法，选择一个subloop管理新channel
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "~%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    // log << new conn

    sockaddr_in local;
    ::bzero(&local, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if (::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0) {
        // error
    }

    InetAddress localAddr(local);

    // 根据连接成功的fd，创建TcpConnection对象
    TcpConnectionPtr conn(new TcpConnection(ioloop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    // 在loop中，完成新连接注册
    ioloop->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    // log
    size_t n = connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connnectionDestroyed, conn));
}