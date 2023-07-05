// #include "thread.h"
#include <thread>
#include <iostream>
#include <vector>
#include <unistd.h>


using namespace std;


void job(){
    pid_t tid = getppid();
    cout << tid << ":in thread\n";
    sleep(1);
}


vector<thread*> vec;
void testThread(int nums){
    for(int i = 0; i < nums; i++){
        thread* thre = new thread(job);
        vec.push_back(thre);
    }
}


int main(){
    testThread(5);
    sleep(1);
    return 0;
}