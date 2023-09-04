#include <stdlib.h>

#include "Poller.h"

// TODO:实现完具体的IO复用类后，再实现返回值
Poller* Poller::newDefaultPoller(EventLoop* loop) {
    // 检查环境变量，在环境变量中选择使用poll/epoll
    if (::getenv("USE_POLL")) {
        return nullptr;
    } else {
        return nullptr;
    }
}