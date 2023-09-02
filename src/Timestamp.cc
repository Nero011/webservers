#include "Timestamp.h"

Timestamp::Timestamp() : microSecond(0) {}
Timestamp::Timestamp(int64_t time) : microSecond(time) {}

Timestamp::~Timestamp() {}

// 返回当前时间
Timestamp Timestamp::now() { return Timestamp(time(NULL)); }

std::string Timestamp::toString() const {
    char buf[128];
    tm* tm_time = localtime(&microSecond);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d", tm_time->tm_year + 1900,
             tm_time->tm_mon + 1, tm_time->tm_mday, tm_time->tm_hour,
             tm_time->tm_min, tm_time->tm_sec);
    return buf;
}

#include <iostream>
int main() { std::cout << Timestamp::now().toString() << std::endl; }