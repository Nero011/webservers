#pragma once

/*
    noncopyable被继承后，派生类不可进行拷贝构造和析构
*/

class noncopyable {
private:
    /* data */
public:
    noncopyable(/* args */);
    ~noncopyable();

    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

noncopyable::noncopyable(/* args */) {}

noncopyable::~noncopyable() {}
