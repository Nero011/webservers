#include "threadPool.h"

using namespace std;

threadPool::threadPool(int threadsNums, task func)
:
    start_(false),
    threadNums_(threadNums_),
    func_(func)
{
    // mutex_ = mutex lock;
    // cond_ = cond cond;
}

threadPool::~threadPool(){
    
}

void threadPool::start(){
    assert(!start_);
    start_ = true;
    for(int i = 0; i < threadNums_; i++){
        thread* th = new thread(func_);
        workThreadList_.push_back(th);
    }
}