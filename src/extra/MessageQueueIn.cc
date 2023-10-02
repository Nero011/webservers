#include "MessageQueueIn.h"

#include <iostream>

#include "RedisConn.h"

void MessageQueueIn::QueueIn(std::string queueName, std::string msg) {
    redisReply* reply;
    reply = (redisReply*)redisCommand(context_->getContext(), "LPUSH %s %s", queueName.c_str(), msg.c_str());
    // redisCommand(context_->getContext(), "set msg 11");
    freeReplyObject(reply);
}
