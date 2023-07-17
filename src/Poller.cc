#include "Poller.h"

// channel的状态值
const int NEW = 0;
const int MOD = 1;
const int DEL = -1;

Poller::Poller(EventLoop *loop):
loop_(loop),
eventListSize_(0)
{
    epollfd_ = epoll_create1(0);
}


Poller::~Poller(){

}


void Poller::updateChannel(Channel& ch){
    if(ch.getStatus() == NEW){
        eventListSize_++;
        update(ch.getEvent(), ch);
    }else if(ch.getEvent() == MOD){
        update(ch.getEvent(), ch);
    }else{  // delete
        update(ch.getEvent(), ch);
        eventListSize_--;
    }
}

void Poller::update(int opertion, Channel& ch){
    struct epoll_event event;
    event.events = ch.getEvent();
    event.data.ptr = &ch;    
    epoll_ctl(epollfd_, opertion, ch.getFd(), &event);
}


void Poller::loop(){
    //FIXME: epollwait添加定时器以及超时处理
    int eventNums = epoll_wait(epollfd_, &*eventList_.begin(), eventListSize_, -1);
    if(eventNums > 0){
        for(int i = 0; i < eventList_.size(); i++){
            Channel* ch = (Channel*)eventList_[i].data.ptr;
            int event = eventList_[i].events;
            if(event == EPOLLIN){
                ch->readCallBack();
            }
            if(event == EPOLLOUT){
                ch->writeCallBack();
            }
            // TODO: 错误处理
        }
    }
}                                                                                                                                                 