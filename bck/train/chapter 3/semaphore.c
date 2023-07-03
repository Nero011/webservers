/*
    生产者消费者模型, 使用信号量
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
 #include <semaphore.h>

#define PROD_MAX_SIZE 100

struct Node{
    int num;
    struct Node* next;
};



struct Node* head = NULL;
int count_node = 0;//链表中总数

// pthread_rwlock_t prod;//使用读写锁
pthread_mutex_t mutex;
sem_t sem_prod;
sem_t sem_cust;
// pthread_cond_t cond;//条件变量

void* customer(void* arg){
    while(1){
        sleep(2);
        // pthread_rwlock_rdlock(&prod);
        pthread_mutex_lock(&mutex);
        printf("%ld cus:\n\n", pthread_self());
        // //使用条件变量实现
        // if(head != NULL){
        //     count_node--;
        //     struct Node* tmp = head;
        //     head = head->next;
        //     printf("customer: %d\ncount: %d\n", tmp->num, count_node);
        //     free(tmp);
        // }else{
        //     printf("waiting...\n");
        //     pthread_cond_wait(&cond, &mutex);

        // }

        //使用信号量
        int val;
        sem_getvalue(&sem_cust, &val);
        printf("sem_cust.val = %d\n", val);
        sem_wait(&sem_cust);
        sem_post(&sem_prod);
        struct Node* tmp = head;
        printf("num: %d\n", head->num);
        head = head->next;
        free(tmp);

        // pthread_rwlock_unlock(&prod);//使用读写锁
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}


void* product(void* arg){
    while(1){
        sleep(1);
        // pthread_rwlock_wrlock(&prod);
        pthread_mutex_lock(&mutex);
        //使用条件变量
        printf("%ld pro:\n\n", pthread_self());
        // if(count_node < 1000){
        //     count_node++;
        //     struct Node* cur = malloc(sizeof(struct Node));
        //     cur->num = rand() % 1000;
        //     cur->next = head;
        //     head = cur;
        //     printf("product %d, count %d\n", head->num, count_node);
        // }

        //使用信号量
        sem_wait(&sem_prod);
        sem_post(&sem_cust);
        struct Node* cur = malloc(sizeof(struct Node));
        cur->num = rand() % 1000;
        cur->next = head;
        head = cur;

        // pthread_rwlock_unlock(&prod);//使用读写锁
        pthread_mutex_unlock(&mutex);
        // pthread_cond_signal(&cond);//使用条件变量
    }
    pthread_exit(NULL);
}

int main(){

    // pthread_rwlock_init(&prod, NULL);//初始化读写锁
    pthread_mutex_init(&mutex,NULL);
    // pthread_cond_init(&cond, NULL);//初始化条件变量
    sem_init(&sem_prod, 0, 100);
    sem_init(&sem_cust, 0, 0);

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

    // pthread_rwlock_destroy(&prod);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&sem_prod);
    sem_destroy(&sem_cust);

    return 0;
}