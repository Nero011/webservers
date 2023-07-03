#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void* callback(void* arg){
    printf("child thread id: %ld\n", pthread_self());
    return NULL;
}


int main(){
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, callback, NULL);
    if (ret != 0){
        char* str = strerror(ret);
        printf("%s\n", str);
    }
    printf("tid = %ld\n", tid);
    printf("main thread id: %ld\n", pthread_self());

    //主线程退出，主线程退出也不会影响其他线程
    pthread_exit(NULL);

    return 0;
}