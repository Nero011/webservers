#include <hiredis/hiredis.h>

#include <iostream>

void producer(redisContext *c, const std::string &message) {
    redisReply *reply = (redisReply *)redisCommand(c, "LPUSH queue %s", message.c_str());
    freeReplyObject(reply);
}

std::string consumer(redisContext *c) {
    redisReply *reply = (redisReply *)redisCommand(c, "BRPOP queue 0");
    std::string message;
    if (reply->type == REDIS_REPLY_ARRAY) {
        message = reply->element[1]->str;
    }
    freeReplyObject(reply);
    return message;
}

int main() {
    // 创建一个Redis连接
    redisContext *c = redisConnect("192.168.2.240", 6379);
    if (c == NULL || c->err) {
        if (c) {
            std::cout << "Error: " << c->errstr << std::endl;
            redisFree(c);
        } else {
            std::cout << "Can't allocate redis context" << std::endl;
        }
        return 1;
    }

    // 生产者发送消息
    producer(c, "Hello, World!");

    // 消费者接收消息
    std::string message = consumer(c);
    std::cout << message << std::endl;

    // 释放Redis连接
    redisFree(c);

    return 0;
}