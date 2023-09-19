#pragma once

#include <hiredis/hiredis.h>

#include "InetAddress.h"
#include "noncopyable.h"

class RedisConn : noncopyable {
private:
    redisContext* conn_;

public:
    RedisConn(InetAddress& addr);
    ~RedisConn();
    void test();
};
