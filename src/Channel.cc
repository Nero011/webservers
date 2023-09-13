#include "Channel.h"

#include <sys/epoll.h>

#include "EventLoop.h"
#include "Logger.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {}

Channel::~Channel() {}

// 绑定弱指针, 用于绑定TcpConnection，在channel执行回调时，通过检测tie_的状态，判断TcpConnnection的状态、
// 如果tie_状态检查失败，意味着绑定的TcpConnection已经失效，就不执行回调
void Channel::tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

// 改变channel fd表示的events事件后,
// 需要update()在poller内更改相应事件的epoll_ctl
// 通过EventLoop调用poller的相应方法，注册事件
void Channel::update() { loop_->updateChannel(this); }

// 在channel所属的event loop中，删除当前的channel
void Channel::remove() { loop_->removeChannel(this); }

void Channel::handlelEvent(Timestamp receiveTime) {
    if (tied_) {                                    // 如果执行过绑定，需要判断
        std::shared_ptr<void> guard = tie_.lock();  // 提升弱指针
        if (guard) {
            handleEventWithGuand(receiveTime);
        }
    } else {  // 没执行过绑定，直接执行回调
        handleEventWithGuand(receiveTime);
    }
}

// 根据poller通知的具体事件，调用相应的回调函数
void Channel::handleEventWithGuand(Timestamp receiveTime) {
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {  // 对端发出RST，并且没有触发EPOLLIN，有异常
        if (closeCallback_) {                              // 如果关闭回调函数存在
            closeCallback_();
        }
    }

    if (revents_ & EPOLLERR) {
        if (errorCallback_) {
            errorCallback_();
        }
    }

    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        if (readCallback_) {
            readCallback_(receiveTime);
        }
    }

    if (revents_ & EPOLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
}