#include "EventLoop.h"


void inConnect(){
    //TODO:处理新连接的函数应该和网络部分放在一起
    return;
}

EventLoop::EventLoop(int listenfd)
{
    poller_ = new Poller;
    listenChannel_ = new Channel(listenfd, EPOLLIN);
    listenChannel_->SetCallBack(READ, inConnect);
}

void EventLoop::start(){
    poller_->update(EPOLL_CTL_ADD, *listenChannel_);
    poller_->loop();
}
