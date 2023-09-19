#include "mymuduo/InetAddress.h"
#include "mymuduo/RedisConn.h"

// g++ -o testRedis test/testRedis.cc -lmymuduo -lpthread -lhiredis -g

int main() {
    InetAddress addr(6379, "192.168.2.240");
    RedisConn conn(addr);
    conn.test();
    return 0;
}