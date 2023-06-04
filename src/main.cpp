#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <signal.h>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"


/**
* @brief          : 注册指定信号捕捉
* @param          : sig: 需要处理的信号， handler：函数指针，处理函数
* @retval         : 无
*/
void addsig(int sig, void(handler)(int)){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
}

int main(int argc, char* argv[]){
    if(argc <= 1){
        printf("please enter %s port_number\n", basename(argv[0]));
        exit(-1);
    }

    //获取端口号
    int port = atoi(argv[1]);

    //对SIGPIPE进行处理
    addsig(SIGPIPE, SIG_IGN);

    //创建线程池
    threadpool<http_conn> *pool = NULL;
    try{
        pool = new threadpool<http_conn>;
    }catch(...){
        exit(-1);
    }

    //创建一个数组保存客户信息

    return 0;
}