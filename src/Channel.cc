#include "Channel.h"



Channel::Channel(int fd, int event, EventLoop* loop):
    fd_(fd),
    event_(event),
    loop_(loop)
{
}

Channel::~Channel()
{
}


