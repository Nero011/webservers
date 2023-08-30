#ifndef CHANNELHEAD
#define CHANNELHEAD
#include <unistd.h>
#include <thread>
#include <sys/epoll.h>

//用于封装文件句柄和事件
enum EVENT{READ = 1, WRITE = 2};

class Channel
{    
public:
    Channel(int fd, int event);
    ~Channel();
    int GetFd() { return _fd;}
    void SetEvent(int event) { _event = event;}
    void SetCallBack(int event, void(*func));

private:
    int _fd;
    int _event;
    void (*_readCall);
    void (*_writeCall);
};





#endif