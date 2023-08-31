#ifndef EVENTLOOPHEAD
#define EVENTLOOPHEAD

//poller的控制器，用于初始化poller和控制poller

#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include "Channel.h"

class Poller{};

class EventLoop{
public:
    EventLoop(int listenfd);
    ~EventLoop();
    void start();

private:
    Poller* poller_;
    Channel* listenChannel_;
    std::vector<Channel> eventList_;


};




#endif
