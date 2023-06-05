#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"


#define MAX_FD 65355//最大文件描述符个数
#define MAX_EVENT_NUM 10000//最大监听事件


extern void addfd(int epollfd, int fd, bool oneshot);//添加文件描述符到epoll中
extern void rmfd(int epollfd, int fd);//从epoll中删除文件描述符
extern void modfd(int epollfd, int fd, int ev);//修改epoll中的文件描述符

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
    http_conn* users = new http_conn[MAX_FD];

    //创建监听socket
    int lfd = socket(PF_INET, SOCK_STREAM, 0);

    //端口复用
    int reuse = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    //绑定
    sockaddr_in address;
    address.sin_family = PF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    bind(lfd, (struct sockaddr*)&address, sizeof(address));

    //监听
    listen(lfd, 5);

    //创建epoll，事件数组，添加监听描述符
    epoll_event events[MAX_EVENT_NUM];
    int epollfd = epoll_create(1);
    http_conn::m_epollfd = epollfd;//给静态成员赋值

    //添加监听文件描述符到epoll
    addfd(epollfd, lfd, false);


    while(true){
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1);
        if((num < 0) && (errno != EINTR)){//返回-1且不是被信号中断后，epollwait不阻塞直接返回
            printf("epoll wait fail\n");
            break;
        }

        //遍历事件数组
        for(int i = 0; i < num; i++){
            int sockfd = events[i].data.fd;


            //监听描述符有事件，表示新客户端连接
            if(sockfd == lfd){
                sockaddr_in client_address;
                socklen_t len = sizeof(client_address);
                int connfd = accept(sockfd, (struct sockaddr*)&client_address, &len);

                if(http_conn::m_user_count >= MAX_FD){
                    //连接数满了
                    /*响应报文*/
                    close(connfd);
                    continue;
                }
                //将新的客户数据放到数组中，并初始化
                users[connfd].init(connfd, client_address);

            }else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                //对方异常断开连接
                users[sockfd].close_conn();//关闭连接
            }else if(events[i].events & EPOLLIN){//读事件
                if(users[sockfd].read()){//一次性把数据读完
                    pool->append(users + sockfd);//从线程池中取一个线程，完成请求
                }else{
                    //读数据失败
                    users[sockfd].close_conn();
                }
            }else if(events[i].events & EPOLLOUT){//写事件
                if(!users[sockfd].write()){//写数据失败
                    users[sockfd].close_conn();
                }
            }


        }
    }



    close(epollfd);
    close(lfd);
    delete[] users;
    delete pool;
    return 0;
}