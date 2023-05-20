/*
    生产者消费者模型
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define PROD_MAX_SIZE 100

int num_prod = 0;
pthread_rwlock_t prod;

void* customer(void* arg){
    while(1){
        sleep(2);
        pthread_rwlock_rdlock(&prod);
        if(num_prod > 0){
            num_prod--;
            printf("customer: %d\n", num_prod);
            printf("===========================\n");
        }
        pthread_rwlock_unlock(&prod);
    }
    pthread_exit(NULL);
}


void* product(void* arg){
    while(1){
        sleep(1);
        pthread_rwlock_wrlock(&prod);
        if(num_prod < 100){
            num_prod++;
            printf("product: %d\n", num_prod);
            printf("===========================\n");
        }
        pthread_rwlock_unlock(&prod);
    }
    pthread_exit(NULL);
}

int main(){

    pthread_rwlock_init(&prod, NULL);//初始化读写锁

    pthread_t prodtid[3];//生产者线程
    pthread_t custtid[3];//消费者线程

    for(int i = 0; i < 3; i++){
        pthread_create(&prodtid[i], NULL, product, NULL);
    }
    for(int i = 0; i < 3; i++){
        pthread_create(&custtid[i], NULL, customer, NULL);
    }

    //回收线程
    for(int i = 0; i < 3; i++){
        pthread_join(prodtid[i], NULL);
    }
    for(int i = 0; i < 3; i++){
        pthread_join(custtid[i], NULL);
    }

    pthread_rwlock_destroy(&prod);

    return 0;
}