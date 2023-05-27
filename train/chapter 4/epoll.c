//epoll实现IO复用

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/epoll.h>


const char *IP = "192.168.88.129";
const int PROT = 8888;



int main(){



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

    //IO多路复用
    //使用epoll
    int epfd = epoll_create(1);//新建epoll实例
    struct epoll_event lfd;
    lfd.events = EPOLLIN;
    lfd.data.fd = fd_s;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd_s, &lfd);

    struct epoll_event events[1024];
    for(int i = 0; i < 1024; i++){
        events[i].data.fd = -1;
    }

    //4.连接
    int nfds = 0;
   
    printf("监听已启动\n");
    while(1){
        ret = epoll_wait(epfd, events, sizeof(events), -1);
        if(ret == -1){
            perror("epoll");
            return -1;
        }
        // printf("epoll返回, ret = %d\n", ret);
        for(int i = 0; i < ret; i++){
            printf("文件描述符：%d\n", events[i].data.fd);
            if(events[i].data.fd == fd_s){ // 新连接
                struct sockaddr_in addr_c;
                addr_c.sin_family = AF_INET;
                socklen_t len = 0;
                int fd_c = accept(fd_s, (struct sockaddr*)&addr_c, &len);
                if (fd_c == -1){
                    perror("accept");
                    return -1;
                }
                printf("客户端已连接！\n");

                //添加新连接进epoll
                struct epoll_event cfd;
                cfd.events = EPOLLIN;
                cfd.data.fd = fd_c;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd_c, &cfd);
                // if(ret != -1) printf("已添加至监听\n");
            }
            else{
                // printf("else?\n");
                // 已连接的客户端
                char buf[1024] = {};
                int num = read(events[i].data.fd, buf, sizeof(buf));
                if(num == -1){
                    perror("read");
                    return -1;
                }else if(num == 0){
                    printf("client close...\n");
                    //清除epoll监听
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
                    close(events[i].data.fd);
                }else{
                    printf("recvbuf: %s\n", buf);
                    write(events[i].data.fd, buf, strlen(buf));
                }
            }
        }
           

        
        


    }
   


    close(fd_s);
    close(epfd);
    

    return 0;
}