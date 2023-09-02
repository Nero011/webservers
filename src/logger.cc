#include "logger.h"

#include <iostream>

Logger::Logger(/* args */) {}

Logger::~Logger() {
    if (instance_ != nullptr) delete instance_;
}

Logger *Logger::getInstance() {
    if (instance_ == nullptr) {
        instance_ = new Logger;
    }
    return instance_;
}

void Logger::setLogType(int level) { logLevel_ = level; }

/// @brief 打印日志，格式为[level] time : msg
/// @param msg
void Logger::logOut(std::string msg) {
    switch (logLevel_) {
        case INFO:
            std::cout << "[INFO]"
                      << " ";
            break;
        case WRINING:
            std::cout << "[WRINING]"
                      << " ";
            break;
        case ERROR:
            std::cout << "[ERROR]"
                      << " ";
            break;
        case FATAL:
            std::cout << "[FATAL]"
                      << " ";
            break;
        case DEBUG:
            std::cout << "[DEBUG]"
                      << " ";
            break;

        default:
            break;
    }
    std::cout << Timestamp::now().toString() << " ";
    std::cout << msg << std::endl;
}