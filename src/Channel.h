#include <unistd.h>
#include <thread>
#include <sys/epoll.h>
#include <boost/function.hpp>

class EventLoop{};    // 空类，不用包含eventloop.h

class Channel
{
public:
    typedef boost::function<void()> EventCallback;
    Channel(int fd, int event, EventLoop* loop);
    ~Channel();
    //TODO:完善eventloop后写update()
    void update();    // 在当前epoll中更新channel数据
    void addReadEvent() { event_ |= EPOLLIN; update(); }
    void rmReadEvent() { event_ &= EPOLLIN; update(); }
    void addWriteEvent() { event_ |= EPOLLOUT; update(); }
    void rmWriteEvent() { event_ &= EPOLLOUT; update(); }
    int getFd() { return fd_; }
    int getEvent() { return event_; }
    void setReadEventCallback(EventCallback cb){
        readEventCallback = std::move(cb);
    }
    void setWriteEventCallback(EventCallback cb){
        writeEventCallback = std::move(cb);
    }
    void setErrEventCallback(EventCallback cb){
        errEventCallback = std::move(cb);
    }

private:
    int fd_;            // 封装的文件句柄
    int event_;         // 需要注册到epoll的事件
    EventLoop *loop_;    // 当前Channel所属的Eventloop
    // handle function
    EventCallback readEventCallback;
    EventCallback writeEventCallback;
    EventCallback errEventCallback;
};
