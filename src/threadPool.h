#ifndef THREADPOOL
#define THREADPOOL
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <boost/function.hpp>

class threadPool
{
public:
    typedef boost::function<void()> task;
    threadPool(int threadsNums, task func);
    ~threadPool();
    void start();
private:
    std::vector<std::thread*> workThreadList_;
    
    std::mutex mutex_;    
    std::condition_variable cond_;
    
    bool start_;
    int threadNums_;
    task func_;
    
};






#endif
