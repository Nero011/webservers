#include "Thread.h"

#include <semaphore.h>

#include "CurrentThread.h"
std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false), joined_(false), tid_(0), func_(std::move(func)), name_(name) {
    setDefaultName();
}
Thread::~Thread() {
    if (started_ && !joined_) {
        thread_->detach();  // thread类提供的线程分离
    }
}

void Thread::start() {
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);

    thread_ = std::shared_ptr<std::thread>(new std::thread([&]() {
        tid_ = CurrentThread::tid();  // 获取线程的tid
        sem_post(&sem);               // 获取到tid了，线程完成初始化，tid可以访问，信号量+1
        func_();                      // 开启新线程，执行线程函数
    }));

    sem_wait(&sem);  // 如果信号量为0，则没完成初始化，start()不能返回
}

void Thread::join() {
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName() {
    int num = ++numCreated_;
    if (name_.empty()) {
        char buf[32] = {};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}