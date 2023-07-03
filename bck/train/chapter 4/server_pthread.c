//多线程实现并发服务器

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>


const char *IP = "192.168.88.129";
const int PROT = 8888;

struct sockInfo{//通信套接字的信息
    int fd;//文件描述符
    struct sockaddr_in addr;//客户端的信息
    pthread_t tid;//线程号
};

struct sockInfo sockInfos[128];
const int MAX = sizeof(sockInfos)/sizeof(sockInfos[0]);



void* comm(void* arg){
    struct sockInfo* pInfo = (struct sockInfo*)arg;
    char ip[16] = {};
    inet_ntop(AF_INET, &pInfo->addr.sin_addr.s_addr, ip, sizeof(ip));
    int prot = ntohs(pInfo->addr.sin_port);
    printf("client:%s %d connected\n", ip, prot);
    char recvbuf[1024] = {};
    while(1){
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = read(pInfo->fd, recvbuf, sizeof(recvbuf));
        if(ret == -1){
            perror("recv");
            pthread_exit(NULL);
        }else if(ret == 0){
            printf("client close...\n");
            pthread_exit(NULL);
        }else if(ret > 0){
            printf("%s %d: %s\n", ip, prot, recvbuf);
        }

        write(pInfo->fd, recvbuf, sizeof(recvbuf));
    }
}

int main(){

    //初始化socketinfo数组
    for(int i = 0; i < MAX; i++){
        bzero(&sockInfos[i], sizeof(sockInfos[i]));
        sockInfos[i].fd = -1;
        sockInfos[i].tid = -1;
    }

    //1.创建socket
    int fd_s = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_s == -1){
        perror("socket");
        return -1;
    }
    //2.绑定IP和端口
    struct sockaddr_in addr_s;
    addr_s.sin_family = AF_INET;
    addr_s.sin_port = htons(PROT);
    inet_pton(AF_INET, IP, &addr_s.sin_addr.s_addr);


    //设置端口复用
    int optval = 1;
    setsockopt(fd_s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


    int ret = bind(fd_s, (struct sockaddr*)&addr_s, sizeof(addr_s));
    if(ret == -1){
        perror("bind");
        return -1;
    }
    //3.监听
    ret = listen(fd_s, 128);
    if(ret == -1){
        perror("listen");
        return -1;
    }

    //4.连接

    //主线程负责连接，子线程负责通信

    struct sockaddr_in addr_c;
    addr_c.sin_family = AF_INET;
    socklen_t len = 0;
    while(1){

        int fd_c = accept(fd_s, (struct sockaddr*)&addr_c, &len);
        if (fd_c == -1){
            perror("accept");
            return -1;
        }



        //连接成功，就创建子线程

        //传入子线程的参数
        struct sockInfo *pInfo = NULL;

        //查找还有没有空余的sockInfo
        for(int i = 0; i < MAX; i++){
            if(sockInfos[i].fd == -1){
                pInfo = &sockInfos[i];
                break;
            }else if(sockInfos[i].fd != -1){
                if(i != MAX -1) continue;
                else {
                    printf("server is busy, please try later\n");
                    break;
                }
            }
        }

        if(pInfo == NULL){
            //队列满了
            continue;
        }
        memcpy(&pInfo->addr, &addr_c, sizeof(addr_c));
        pInfo->fd = fd_c;


        pthread_create(&pInfo->tid, NULL, comm, (void*)pInfo);

        //线程分离
        pthread_detach(pInfo->tid);

    }
    close(fd_s);

}