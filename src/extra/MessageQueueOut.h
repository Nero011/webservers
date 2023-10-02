#pragma once
#include <hiredis/hiredis.h>

#include <string>

#include "noncopyable.h"

class RedisConn;

// 用redis实现的消息队列出口
class MessageQueueOut : noncopyable {
private:
    RedisConn* context_;

public:
    MessageQueueOut(RedisConn* context) : context_(context) {}
    ~MessageQueueOut() {}

    std::string getMessage(std::string queueName);
};
