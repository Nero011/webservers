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


// void Poller::updateChannel(Channel& ch){
//     if(ch.getStatus() == NEW){
//         eventListSize_++;
//         update(ch.GetEvent(), ch);
//     }else if(ch.GetEvent() == MOD){
//         update(ch.GetEvent(), ch);
//     }else{  // delete
//         update(ch.GetEvent(), ch);
//         eventListSize_--;
//     }
// }

void Poller::update(int opertion, Channel& ch){
    struct epoll_event event;
    if(ch.GetEvent() == READ) event.events = EPOLLIN;
    else if(ch.GetEvent() == WRITE) event.events = EPOLLOUT;
    event.data.ptr = &ch;    
    epoll_ctl(epollfd_, opertion, ch.GetFd(), &event);
}


void Poller::loop(){
    //FIXME: epollwait添加定时器以及超时处理
    int eventNums = epoll_wait(epollfd_, &*eventList_.begin(), eventListSize_, -1);
    if(eventNums > 0){
        for(int i = 0; i < eventList_.size(); i++){
            Channel* ch = (Channel*)eventList_[i].data.ptr;
            int event = eventList_[i].events;
            if(event == EPOLLIN){
                ch->ReadCallBack();
            }
            if(event == EPOLLOUT){
                ch->WriteCallBack();
            }
            // TODO: 错误处理
        }
    }
}                                                                                                                                                 