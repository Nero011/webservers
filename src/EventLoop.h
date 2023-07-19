#ifndef EVENTLOOPHEAD
#define EVENTLOOPHEAD


#include <vector>
#include "Channel.h"
#include "Poller.h"

class EventLoop{
public:
    EventLoop();
    ~EventLoop();

private:
    Poller poller_;
    Channel listenChannel_;
    std::vector<Channel> eventList_;


};




#endif
