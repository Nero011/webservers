#include "threadPool.h"

using namespace std;

threadPool::threadPool(int threadsNums, task func)
:
    start_(false),
    threadNums_(threadNums_),
    func_(func)
{    
    
    
}

threadPool::~threadPool(){
    for(int i = 0; i < threadNums_; i++){
        if(!workThreadList_[i]->joinable()){
            workThreadList_[i]->detach(); 
            cout << "tid:" << workThreadList_[i]->get_id() << "detach!" << endl;
            
        } 
    }    
}

void threadPool::start(){
    assert(!start_);
    start_ = true;
    for(int i = 0; i < threadNums_; i++){
        thread* th = new thread(func_);
        workThreadList_.push_back(th);    
    }
}
