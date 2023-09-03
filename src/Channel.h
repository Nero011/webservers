#pragma once

#include <functional>
#include <memory>

#include "Timestamp.h"
#include "noncopyable.h"
// 用于封装文件句柄和事件

class
    EventLoop;  // 前置声明，头文件中只需要指针，需要用到方法可以在源文件中包含

class Channel : noncopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    /// @brief  得到poller通知后，调用回调，处理事件
    /// @param receiveTime: 接受到任务的时间
    void handlelEvent(Timestamp receiveTime);
    void setReadCallback(ReadEventCallback cb) {
        readCallback_ = std::move(cb);
    }
    void setWirteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止当channel被手动remove后，还在执行回调操作
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }  // 供poller调用

    // 使能/取消channel的感兴趣事件
    void enableReading() {
        events_ |= kReadEvent;
        update();
    }
    void disableReading() {
        events_ &= ~kReadEvent;
        update();
    }
    void disableWriting() {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll() {
        events_ = kNoneEvent;
        update();
    }
    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isReading() const { return events_ & kReadEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }

    int index() { return index_; }
    void setIndex(int index) { index_ = index; }

    // 多线程
    EventLoop *ownerLoop() { return loop_; }
    void remove();

private:
    void update();  // 更新poller
    void handleEventWithGuand(Timestamp receiveTime);

    // 当前含兴趣事件
    static const int kNoneEvent;   // 没有事件
    static const int kReadEvent;   // 读事件
    static const int kWriteEvent;  // 写事件

    EventLoop *loop_;  // 事件循环
    const int fd_;     // 文件句柄
    int events_;       // 注册fd感兴趣的事件
    int revents_;      // poller返回的具体发生的事件
    int index_;

    // 用于绑定自身的弱指针，使用时先提升弱指针，提升成功证明对象存活
    std::weak_ptr<void> tie_;
    bool tied_;

    // channel能获知fd最终发生的事件，所以由channel来执行回调
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
