//定时器练习
//延时3秒，2秒钟定时一次
#include <stdio.h>
#include <sys/time.h>

int main(){
    //定时器结构体，设置延时3秒，2秒定时一次
    struct itimerval timer;
    timer.it_interval.tv_sec = 2;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 3;
    timer.it_value.tv_usec = 0;

    //启用定时器，以真实时间计时
    int ret = setitimer(ITIMER_REAL, &timer, NULL);
    if(ret == -1){
        perror("setitimer");
        return -1;
    }



    return 0;
}