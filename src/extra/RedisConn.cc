#include "RedisConn.h"

#include <iostream>

RedisConn::RedisConn(InetAddress& addr) {
    conn_ = redisConnect(addr.toIp().c_str(), addr.toPort());
    redisCommand(conn_, "auth msgstart");  // 输入密码
}
RedisConn::~RedisConn() { redisFree(conn_); }
