#ifndef CHANNELHEAD
#define CHANNELHEAD
#include <unistd.h>
#include <thread>
#include <sys/epoll.h>

//用于封装文件句柄和事件
enum EVENT{READ = 1, WRITE = 2};
//enum STATUS{NEW = 0, MOD = 1, DEL = -1};


class Channel
{    
public:
    Channel(int fd, int event);
    ~Channel();
    int GetFd() { return _fd;}
    int GetEvent() { return _event;}
    void SetEvent(int event) { _event = event;}
    void SetCallBack(int event, void(*func));
    void ReadCallBack(){ _readCall;}
    void WriteCallBack(){ _writeCall;}

private:
    int _fd;
    int _event;
    void (*_readCall);
    void (*_writeCall);
};





#endif