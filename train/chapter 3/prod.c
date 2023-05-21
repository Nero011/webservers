/*
    生产者消费者模型
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define PROD_MAX_SIZE 100

struct Node{
    int num;
    struct Node* next;
};



struct Node* head = NULL;
int count_node = 0;//链表中总数

pthread_rwlock_t prod;
pthread_mutex_t mutex;
pthread_cond_t cond;

void* customer(void* arg){
    while(1){
        sleep(1);
        // pthread_rwlock_rdlock(&prod);
        pthread_mutex_lock(&mutex);
        printf("%ld cus:\n\n", pthread_self());
        if(head != NULL){
            count_node--;
            struct Node* tmp = head;
            head = head->next;
            printf("customer: %d\ncount: %d\n", tmp->num, count_node);
            free(tmp);
        }else{
            printf("waiting...\n");
            pthread_cond_wait(&cond, &mutex);

        }
        // pthread_rwlock_unlock(&prod);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}


void* product(void* arg){
    while(1){
        usleep(5000);
        // pthread_rwlock_wrlock(&prod);
        pthread_mutex_lock(&mutex);
        printf("%ld pro:\n\n", pthread_self());
        if(count_node < 1000){
            count_node++;
            struct Node* cur = malloc(sizeof(struct Node));
            cur->num = rand() % 1000;
            cur->next = head;
            head = cur;
            printf("product %d, count %d\n", head->num, count_node);
        }
        // pthread_rwlock_unlock(&prod);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    }
    pthread_exit(NULL);
}

int main(){

    pthread_rwlock_init(&prod, NULL);//初始化读写锁
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond, NULL);

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