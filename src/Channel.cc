#include "Channel.h"

const int NEW = 0;
const int MOD = 1;
const int DEL = -1;

Channel::Channel(int fd, int event, EventLoop* loop):
    fd_(fd),
    event_(event),
    loop_(loop),
    status_(NEW)
{
}

Channel::~Channel()
{
}

void Channel::update(){
    //TODO: 在eventloop中收到channel的更新请求，然后由eventloop调用poller更新channel
    loop_.updateChannel(this);
}


