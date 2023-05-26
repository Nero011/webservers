//select实现IO复用

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>


const char *IP = "192.168.88.129";
const int PROT = 8888;



int main(){

    //IO多路复用
    fd_set reads;//要检测的已连接客户端文件描述符集合
    //初始化fd_set
    FD_ZERO(&reads);

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
    //没有子线程/子进程了
    FD_SET(fd_s, &reads);
    int maxfd = fd_s;
    printf("监听已启动\n");
    while(1){
        fd_set temp = reads;
        int ret = select(maxfd+1, &temp, NULL, NULL, NULL);
        if(ret == -1){
            perror("select");
            return -1;
        }
        if(FD_ISSET(fd_s, &temp)){
            //有新的客户端连接
            struct sockaddr_in addr_c;
            addr_c.sin_family = AF_INET;
            socklen_t len = 0;
            int fd_c = accept(fd_s, (struct sockaddr*)&addr_c, &len);
            if (fd_c == -1){
                perror("accept");
                return -1;
            }
            printf("客户端已连接！\n");
            FD_SET(fd_c, &reads);
            maxfd = maxfd > fd_c ? maxfd : fd_c;
        }
        //判断已连接的客户端
        for(int i = fd_s + 1; i <= maxfd; i++){
            if(FD_ISSET(i, &temp)){
                char buf[1024] = {};
                int num = read(i, buf, sizeof(buf));
                if(num == -1){
                    perror("read");
                    return -1;
                }else if(num == 0){
                    printf("client closed...\n");
                    FD_CLR(i, &reads);

                }else{
                    printf("recvbuf: %s\n", buf);
                    write(i, buf, strlen(buf));
                }

            }
        }

    }
   

    

    return 0;
}