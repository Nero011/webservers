#include "Poller.h"

#include "Channel.h"

Poller::Poller(EventLoop* loop) : ownLoop_(loop) {}

bool Poller::hasChannel(Channel* channel) const {
    auto it = channels_.find(channel->fd());
    // 当在map内找到了，而找到的和传入的相同
    return it != channels_.end() && it->second == channel;
}