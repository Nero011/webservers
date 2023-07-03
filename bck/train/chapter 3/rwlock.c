//读写锁
//8个线程
//3个线程不定时写，5个线程不定时读



#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


int num = 1000;
pthread_rwlock_t rwlock;

void* writenum(void* arg){
    
    
    while(1){
        // usleep(100);
        pthread_rwlock_wrlock(&rwlock);
        if(num > 0){
            num--;
            printf("write, tid: %ld, num: %d\n", pthread_self(), num);
            pthread_rwlock_unlock(&rwlock);
        }
        else{
            pthread_rwlock_unlock(&rwlock);
            break;
        }
    }


    return NULL;
}

void* readnum(void* arg){

    while(1){
        // usleep(100);
        pthread_rwlock_rdlock(&rwlock);
        if(num > 0){
            printf("read, tid: %ld, num: %d\n", pthread_self(), num);
            pthread_rwlock_unlock(&rwlock);
        }
        else{
            pthread_rwlock_unlock(&rwlock);
            break;
        }
    }


    return NULL;
}


int main(){

    pthread_rwlock_init(&rwlock, NULL);

    pthread_t wrtid[3];
    pthread_t rdtid[5];

    for(int i = 0; i < 3; i++){
        pthread_create(&wrtid[i], NULL, writenum, NULL);
    }
    for(int i = 0; i < 5; i++){
        pthread_create(&rdtid[i], NULL, readnum, NULL);
    }

    
    //线程回收
    for(int i = 0; i < 3; i++){
        pthread_join(wrtid[i], NULL);
    }
    for(int i = 0; i < 5; i++){
        pthread_join(rdtid[i], NULL);
    }



    pthread_rwlock_destroy(&rwlock);
    pthread_exit(NULL);

    return 0;
}