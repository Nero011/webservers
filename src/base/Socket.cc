#include "Socket.h"

#include <fcntl.h>
#include <netinet/tcp.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "InetAddress.h"
#include "Logger.h"
Socket::~Socket() { close(sockfd_); }

void Socket::bindAddress(const InetAddress &locoladdr) {
    if (0 != bind(sockfd_, (sockaddr *)locoladdr.getSockAddr(), sizeof(sockaddr))) {
        // LOG << FATAL;
        // exit;
    }
}
void Socket::listen() {
    if (0 != ::listen(sockfd_, 1024)) {
        // LOG << FATAL;
        // exit;
    }
}

// 用于客户端
int Socket::connect(const InetAddress &addr) {
    int fd = ::connect(sockfd_, (sockaddr *)addr.getSockAddr(), sizeof(sockaddr_in));
    return fd;
}

/// @brief accept的封装
/// @param peeraddr 传出参数，接受到了连接地址
/// @return 接受到的新fd
int Socket::accept(InetAddress *peeraddr) {
    sockaddr_in addr;
    bzero(&addr, sizeof addr);
    socklen_t len = sizeof(addr);  // len必须初始化
    int connfd = ::accept(sockfd_, (sockaddr *)&addr, &len);

    if (connfd >= 0) {
        peeraddr->setSockAddr(addr);
    }
    ::fcntl(connfd, O_NONBLOCK);  // 修改为非阻塞io
    return connfd;
}

// 关闭写端，半断开
void Socket::shutdownWrite() {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        // LOG << ERROR;
    }
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}
void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}
void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}
void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}