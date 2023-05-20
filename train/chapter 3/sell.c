/*
多线程实现卖票案例

    三个窗口，共100张票
*/


#include<stdio.h>
#include<pthread.h>


int ticket = 100;//票的总数

void* sellticket(void* arg){

    //卖票
    while(ticket > 0){
        printf("%ld 还剩%d张票\n", pthread_self(), ticket);
        ticket--;
    }
    return NULL;
}

int main(){

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


    return 0;

}