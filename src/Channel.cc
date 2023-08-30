#include "Channel.h"

//event: EPOLLIN/EPOLLOUT
Channel::Channel(int fd, int event):
_fd(fd),
_event(event)
{

}

void Channel::SetCallBack(int event, void(*func)){
   if(event == READ) {
    _readCall = func;
   }else if(event == WRITE){
    _writeCall = func;
   }
}

