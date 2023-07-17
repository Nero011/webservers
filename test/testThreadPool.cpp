#include "threadPool.h"
#include <iostream>
#include <condition_variable>

void work(){
    
}

int main(){
    threadPool pool(5, work);
    pool.start();

}