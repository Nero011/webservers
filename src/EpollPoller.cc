#include "EpollPoller.h"

#include <unistd.h>

#include <cstring>

#include "Channel.h"
#include "Logger.h"

// channel的index_变量，表示当前在poller中的状态
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        // log << "epoll_create error";
    }
}

EpollPoller::~EpollPoller() { ::close(epollfd_); }

/// @brief 执行epoll_wait
/// @param timeoutMs 超时时间
/// @param activeChannels 活跃的通道
/// @return 时间戳
Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    // log debug <<

    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());  // 获取当前时间

    if (numEvents > 0) {
        // log debug << " "
        fillActiveChannel(numEvents, activeChannels);
        if (numEvents == events_.size()) {  // 返回的事件数量和vector的长度一样，要扩容
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        // 超时
        // log <<
    } else {
        if (saveErrno != EINTR) {  // 不是由外部中断引起的错误，证明出错
            errno = saveErrno;
            // log error << ;
        }
    }
}

// 把具体发生的事件，填写回绑定的channel中
void EpollPoller::fillActiveChannel(int numEvents, ChannelList* activeChannels) const {
    Channel* channel;
    for (int i = 0; i < numEvents; ++i) {
        channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);  // 这样activeChannels就拿到了活跃的通道
    }
}

// channel::update() -> Eventloop::updateChannel() -> this

/// @brief 根据channel->index_的状态，选择添加/更新等操作
/// @param channel
void EpollPoller::updateChannel(Channel* channel) {
    const int index = channel->index();
    // log << fd << events << index

    if (index == kNew || index == kDeleted) {
        if (index == kNew) {  // 如果是新的channel，首先添加到map
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {  // 已经在epoll上注册了
        int fd = channel->fd();
        if (channel->isNoneEvent()) {  // 如果不对任何事件感兴趣
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    int index = channel->index();
    channels_.erase(fd);    // 从map中删除
    if (index == kAdded) {  // 如果是以添加的话，需要从epoll中移除
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

// 执行epoll_ctl
void EpollPoller::update(int operation, Channel* channel) {
    epoll_event event;
    int fd = channel->fd();
    memset(&event, 0, sizeof(event));
    event.data.fd = fd;
    event.events = channel->events();
    event.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {  // 注册失败
        if (operation == EPOLL_CTL_DEL) {
            // 只是删除的话，程序可以正常运行，不用退出
            // log <<
        } else {
            // log FATAL <<
        }
    }
}
