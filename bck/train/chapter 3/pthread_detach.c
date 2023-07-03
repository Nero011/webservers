//分离线程

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void* callback(){
    printf("子线程\n");
}

int main(){

    pthread_t tid;

    pthread_create(&tid, NULL, callback, NULL);

    printf("main: %ld\ntid: %ld\n", pthread_self(), tid);

    //子线程分离
    pthread_detach(tid);

    pthread_exit(NULL);

}