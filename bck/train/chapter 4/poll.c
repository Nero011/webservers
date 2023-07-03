//poll实现IO复用

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <poll.h>


const char *IP = "192.168.88.129";
const int PROT = 8888;



int main(){

    //IO多路复用
    //使用poll
    struct pollfd fds[1024];
    for(int i = 0; i < 1024; i++){
        fds[i].fd = -1;
        fds[i].events = POLLIN;
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

    fds[0].fd = fd_s;//设置监听文件描述符

    //4.连接
    int nfds = 0;
   
    printf("监听已启动\n");
    while(1){
        ret = poll(fds, nfds + 1, -1);
        if(ret == -1){
            perror("poll");
            return -1;
        }else if(ret > 0){
            //新连接处理
            if(fds[0].revents & POLLIN){
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
                int ISBUSY = 0;//连接数是否已满
                for(int i = 1; i < 1024; i++){//把新连接加入尚未初始化的fds中
                    if(fds[i].fd == -1){
                        fds[i].fd = fd_c;
                        nfds = nfds > fd_c ? nfds : fd_c;
                        break;
                    }
                    if(i == 1024-1 && fds[i].fd != -1){
                        ISBUSY = 1;//服务器连接数已满
                    }

                }
                //连接已满，发送信息后断开连接
                char busy[] = "server is busy, please try later.\n";
                // if(ISBUSY == 1){
                //     write(fd_c, busy, sizeof(busy));
                //     close(fd_c);
                // }
                

            }
            //已建立的连接
            for(int i = 1; i < 1024; i++){
                if(fds[i].fd != -1 && fds[i].revents & POLLIN){
                    char buf[1024] = {};
                    int num = read(fds[i].fd, buf, sizeof(buf));
                    if(num == -1){
                        perror("read");
                        return -1;
                    }else if(num == 0){
                        printf("client close...\n");
                        close(fds[i].fd);
                        fds[i].fd = -1;
                    }else{
                        printf("recvbuf: %s\n", buf);
                        write(fds[i].fd, buf, strlen(buf));
                    }

                }
            }

        }
        


    }
   

    

    return 0;
}