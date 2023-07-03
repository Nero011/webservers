//创建一个子线程




#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void* callback(void * arg){
    printf("child pthread running...\n");
    printf("%s", (char*)arg);
    return NULL;
}

int main(){

    pthread_t tid;
    char* arg = "this is ARG\n";


    int ret = pthread_create(&tid, NULL, callback, (void*)arg);
    if(ret != 0){
        char* strerr = strerror(ret);
    }

    sleep(2);

    return 0;
}