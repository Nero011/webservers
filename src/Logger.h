#pragma once
/*
    单例模式的日志
*/
#include <Timestamp.h>

#include <string>

// 日志级别
enum { INFO, WRINING, ERROR, FATAL, DEBUG };

class Logger {
private:
    Logger();
    static Logger *instance_;  // 保存的单例
    int logLevel_;             // 日志级别
public:
    static Logger *getInstance();  // 获取单例
    void setLogType(int);          // 设置日志级别
    void logOut(std::string msg);
    ~Logger();
};
