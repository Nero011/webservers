#include <iostream>

#include "mymuduo/InetAddress.h"
#include "mymuduo/MessageQueueIn.h"
#include "mymuduo/MessageQueueOut.h"
#include "mymuduo/RedisConn.h"
// g++ -o testRedis test/testRedis.cc -lmymuduo -lpthread -lhiredis -g

using namespace std;
int main() {
    InetAddress addr(6379, "192.168.2.240");
    RedisConn conn(addr);

    MessageQueueIn input(&conn);
    MessageQueueOut output(&conn);

    input.QueueIn("asdf", "test in");

    cout << output.getMessage("asdf");
    // redisContext* conn = redisConnect("192.168.2.240", 6379);
    // redisReply* reply1 = (redisReply*)redisCommand(conn, "auth msgstart");
    // cout << " 1";
    // freeReplyObject(reply1);
    // redisCommand(conn, "set msg 11");
    // redisReply* reply = (redisReply*)redisCommand(conn, "get msg");

    // cout << reply->str;

    return 0;
}