#include "EventLoop.h"

#include <fcntl.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "Channel.h"
#include "Poller.h"

// 防止一个线程创建多个loop
__thread EventLoop* t_loopInThisThread = nullptr;

// 默认的默认Poller超时时间，10s
const int kPollerTimeMs = 10000;

// 创建wakeupfd，用于通知subReactor
int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        // log FATAL <<
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_))

{
    // log DEBUG <<
    if (t_loopInThisThread) {  // 当前线程已经有loop了，退出
        // log FATAL
        exit(0);
    } else {
        t_loopInThisThread = this;
    }

    // 设置wakeupfd的事件类型以及回调
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个eventloop都监听wakeupChannel的EPOLLIN事件
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

// read的封装，读wakeupfd上的事件
void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        // log << err
    }
}

// 开启事件循环
void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    // log << loop start
    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollerTimeMs, &activeChannels_);
        for (Channel* channel : activeChannels_) {
            // Poller检测到channel发生事件了，上报给EventLoop，通知channel处理事件
            channel->handlelEvent(pollReturnTime_);
        }
        // 执行本EventLoop要执行的回调操作，比如当前Loop是mainLoop,
        // 在执行完每个channel的事件后，需要分发channel，这个时候就事先把分发注册到loop的
        // pendingFunctors_中，然后在doPendingFunctors()中调用
        doPendingFunctors();
    }
    // logINFO<<
    looping_ = false;
}

// 退出事件循环
void EventLoop::quit() {
    quit_ = true;

    // 如果不是在loop线程中，调用了quit, 那么首先要用wakeup()，把loop线程从epoll_wait中返回，然后才能退出while循环
    if (!isInLoopThread()) {
        wakeup();
    }
}

// 在当前loop线程中执行回调
void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

// 通知相应的loop执行回调
void EventLoop::queueInLoop(Functor cb) {
    {  // 创建一个作用域，在离开这个作用域时，mutex自动解锁
        std::unique_lock<std::mutex>(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    // 再次检测是否在绑定的线程中，或者正在进行回调
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

// 唤醒线程，其实就是让线程从epoll_wait状态中退出
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n < sizeof one) {
        // logERROR <<
    }
}

void EventLoop::updateChannel(Channel* channel) { poller_->updateChannel(channel); }
void EventLoop::removeChannel(Channel* channel) { poller_->removeChannel(channel); }
bool EventLoop::hasChannel(Channel* channel) { return poller_->hasChannel(channel); }

// 执行当前线程的回调
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functor;
    callingPendingFunctors_ = true;
    {
        // 使用swap，一次性取出队列中的回调，减少锁占用时间
        std::unique_lock<std::mutex>(mutex_);
        functor.swap(pendingFunctors_);
    }
    for (const Functor cb : functor) {
        cb();
    }
    callingPendingFunctors_ = false;
}