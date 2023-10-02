#pragma once
// 消息队列出口

#include <hiredis/hiredis.h>

#include <string>

#include "noncopyable.h"

class RedisConn;

class MessageQueueIn : noncopyable {
private:
    RedisConn* context_;

public:
    MessageQueueIn(RedisConn* context) : context_(context) {}
    ~MessageQueueIn() {}
    void QueueIn(std::string queueName, std::string msg);
};
