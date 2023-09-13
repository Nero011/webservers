#include "TcpConnection.h"

#include <functional>
#include <memory>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include "Socket.h"

static EventLoop* CheckLoopNotNull(EventLoop* loop) {
    if (loop == nullptr) {
        // LOG FATAL
        exit(0);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& nameArg, int sockfd, const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(CheckLoopNotNull(loop))
    , name_(nameArg)
    , state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64 * 1024 * 1024)  // 64M
{
    // 这里给channel设置相应的回调函数
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWirteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    socket_->setKeepAlive(true);
}
TcpConnection::~TcpConnection() {
    // log
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        // 已建立连接的用户，有可读事件发生了，调用用户传入的回调函数
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        // 连接断开
        handleClose();
    } else {
        // 错误
        errno = savedErrno;
        // log error <<
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {  // 是否可写？
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    // 此时上层已经调用关闭，直接shutdown
                    shutdownInLoop();
                }
            }
        }
    } else {
        // 不可写
        // log << errno
    }
}

void TcpConnection::handleClose() {
    // log
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}

void TcpConnection::handleError() {
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    } else {
        err = optval;
    }
    // log error
}

void TcpConnection::send(const std::string& buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        } else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}
// 发送数据，应用写得快，而内核发送慢，需要把发送数据写入缓冲区，并且设置水位回调
void TcpConnection::sendInLoop(const void* message, size_t len) {
    ssize_t nwrote = 0;      // 本次发送的数据长度
    size_t remaining = len;  // 剩余未发送的长度
    bool faultError = false;

    if (state_ == kDisconnected) {
        // 调用过shutdown
        // log << error
        return;
    }

    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        // channel第一次写数据，且缓冲区没有待发送数据
        nwrote = ::write(channel_->fd(), message, len);
        remaining = len - nwrote;
        if (nwrote >= 0) {                                   // 发送成功
            if (remaining == 0 && writeCompleteCallback_) {  // 一次性发完数据，而且有注册回调
                // 直接执行回调，不用注册EPOLL_OUT事件，也不用调用handleWrite()
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }  // NOTE:为什么没发完的处理不放在这
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                // log << write error
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    if (faultError && remaining > 0) {
        // 这次write没有把数据发完，需要保存在缓冲区中，然后注册EPOLL_OUT、
        // 此处用readableBytes()返回的是可读区域，但是在outputBuffer中，
        // 只有来不及发送完的数据才会写入，只有下一次发送时，才读取outputBuff
        // 所以可读区域的起始位置，就是下一次接着发送的起始位置
        size_t old_len = outputBuffer_.readableBytes();
        if (old_len + remaining >= highWaterMark_ && old_len < highWaterMark_ && highWaterMarkCallback_) {
            loop_->queueInLoop(
                std::bind(TcpConnection::highWaterMarkCallback_, shared_from_this(), old_len + remaining));
            outputBuffer_.append((char*)message + nwrote, remaining);  // 将未发送的数据添加到缓冲区
            if (!channel_->isWriting()) {
                channel_->enableWriting();  // 让channel对epollout感兴趣
            }
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}
void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {  // 说明当前已经没有要发送的数据了
        socket_->shutdownWrite();  // 关闭写端，触发EPOLLHUP，poller中的epoll返回,
                                   // 然后触发channel的事件处理回调，然后执行注册的closecallback()
    }
}

// 创建连接时调用
void TcpConnection::connectionEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();  // 向poller注册epollin

    connectionCallback_(shared_from_this());  // 执行新连接建立的回调
}

// 断开连接时调用
void TcpConnection::connnectionDestroyed() {
    if (state_ = kConnected) {
        setState(kDisconnected);
        channel_->disableAll();

        closeCallback_(shared_from_this());
    }
    channel_->remove();  // 从poller中删除
}