#pragma once

#include <hiredis/hiredis.h>

#include "InetAddress.h"
#include "noncopyable.h"

class RedisConn : noncopyable {
private:
    redisContext* context_;

public:
    RedisConn(InetAddress& addr);
    ~RedisConn();
    redisContext* getContext() const { return context_; }
};
