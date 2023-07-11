#include <mutex>
#include <thread>
#include <iostream>

using namespace std;
int main(){
    mutex mutex_;
    lock_guard<mutex> guard(mutex_);
    
    return 0;
}
