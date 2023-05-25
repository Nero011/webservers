//多进程实现并发

#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>
#include <stdlib.h>

//服务器地址和端口i
const char* IP = "192.168.88.129";
const int PROT = 8888;

void recyleChild(int arg){
    while(1){
        int ret = waitpid(-1, NULL, WNOHANG);
        if(ret == -1){
            //所有子进程已经退出
            break;
        }else if(ret == 0)
        {
            //还有子进程存活
            break;
        }else if(ret > 0){
            printf("子进程%d被回收\n", ret);
        }
    }
}

int main(){
    //注册信号捕捉，用于回收子进程
    struct sigaction act;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = recyleChild;
    sigaction(SIGCHLD, &act, NULL);



    //1.建立socket
    int fd_s = socket(AF_INET, SOCK_STREAM, 0);//ipv4，流传输，TCP协议
    if(fd_s == -1){
        perror("socket");
        return -1;
    }

    //2.绑定socket
    struct sockaddr_in addr_s;
    addr_s.sin_family = AF_INET;
    addr_s.sin_port = htons(PROT);
    inet_pton(AF_INET, IP, &addr_s.sin_addr.s_addr);
    int ret = bind(fd_s, (struct sockaddr*)&addr_s, sizeof(addr_s));
    if(ret == -1){
        perror("bind");
        return -1;
    }

    //3.监听
    ret = listen(fd_s, 128);//最大等待队列+已连接队列：128
    if(ret == -1){
        perror("listen");
        return -1;
    }


    //4.等待连接
    struct sockaddr_in addr_c;
    addr_c.sin_family = AF_INET;
    socklen_t len = 0;

    while(1){
        int fd_c = accept(fd_s, (struct sockaddr*)&addr_c, &len);
        if(fd_c == -1){
            perror("accept");
            if(errno == EINTR){
                // perror("软中断引起的调用错误");
                continue;
            }
            return -1;
        }

        
        pid_t pid = fork();
        if(pid == 0){
            char ip_client[16];
            inet_ntop(AF_INET, &addr_c.sin_addr.s_addr, ip_client, sizeof(ip_client));
            printf("客户端已连接！\n%s: %d\n", ip_client, ntohs(addr_c.sin_port));

            char recvbuf[1024];
            while(1){
                memset(recvbuf, 0, sizeof(recvbuf));
                int recvlen = read(fd_c, recvbuf, sizeof(recvbuf));
                if(recvlen == -1){
                    perror("recv");
                    exit(-1);
                }else if(recvlen == 0){
                    printf("client close...\n");
                    exit(0);
                }else{
                    printf("%s  %d: %s\n", ip_client, ntohs(addr_c.sin_port), recvbuf);
                }
                write(fd_c, recvbuf, strlen(recvbuf));
            }
        }
    }

    return 0;

}