#include <iostream>
#include "src/threadPool.h"

using namespace std;

void job(){
    
    cout << this_thread::get_id() << ":in thread\n";
    sleep(1);
}

void testPool(){
    threadPool pool(5, job);
    pool.start();
}

int main(){
    testPool();
    sleep(1);
    return 0;
}