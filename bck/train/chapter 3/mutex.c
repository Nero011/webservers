/*
多线程实现卖票案例

    三个窗口，共100张票
*/


#include<stdio.h>
#include<pthread.h>


int ticket = 1000;//票的总数

//在全局创建互斥量
pthread_mutex_t mutex;


void* sellticket(void* arg){

    //卖票
    while(1){
        pthread_mutex_lock(&mutex);
        if(ticket > 0){
            //在临界区加锁
            printf("%ld 还剩%d张票\n", pthread_self(), ticket);
            ticket--;
        }
        else{
            pthread_mutex_unlock(&mutex);   
            break;
        }
        pthread_mutex_unlock(&mutex);
    }
    //操作完成后解锁

    return NULL;
}

int main(){

    //初始化互斥锁
    pthread_mutex_init(&mutex, NULL);


    //创建子线程
    pthread_t window1;
    pthread_t window2;
    pthread_t window3;

    pthread_create(&window1, NULL, sellticket, NULL);
    pthread_create(&window2, NULL, sellticket, NULL);
    pthread_create(&window3, NULL, sellticket, NULL);

    //分离子线程
    pthread_detach(window1);
    pthread_detach(window2);
    pthread_detach(window3);



    //退出主线程
    pthread_exit(NULL);


    //销毁锁
    while(ticket > 0){

    }
    pthread_mutex_destroy(&mutex);

    return 0;

}