#include "MessageQueueOut.h"

#include <iostream>

#include "RedisConn.h"

std::string MessageQueueOut::getMessage(std::string queueName) {
    redisReply* reply;
    reply = (redisReply*)redisCommand(context_->getContext(), "BRPOP %s %d", queueName.c_str(), (size_t)0);
    // reply = (redisReply*)redisCommand(context_->getContext(), "get msg");
    // reply = (redisReply*)redisCommand(context_->getContext(), "BRPOP msg 0");

    std::string message;
    // if (reply->type == REDIS_REPLY_ARRAY) {
    // message = reply->element[1]->str;
    message = reply->str;
    // std::cout << reply->elements;
    // }
    freeReplyObject(reply);
    return message;
}
