#ifndef THREADPOOL
#define THREADPOOL

#include <thread>
#include <vector>
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
    // TODO: mutex and conditional
    // mutex mutex_;
    // cond cond_;
    bool start_;
    int threadNums_;
    task func_;
    
};






#endif