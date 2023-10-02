#include "RedisConn.h"

#include <iostream>

RedisConn::RedisConn(InetAddress& addr) {
    context_ = redisConnect(addr.toIp().c_str(), addr.toPort());
    redisReply* reply = (redisReply*)redisCommand(context_, "auth msgstart");  // 输入密码
    // if (reply->str == "OK") {
    //     return;
    // } else {
    //     std::cout << reply->type << " " << reply->str;
    //     printf("err conn");
    //     exit(0);
    // }
    freeReplyObject(reply);
}
RedisConn::~RedisConn() { redisFree(context_); }
