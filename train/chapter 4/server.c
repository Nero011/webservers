//服务器，简易版
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

const char *IP = "192.168.88.129";//ip地址
const short PROT = 8888;//端口

int main(){


    //1.创建套接字，用于监听
    int fd_lis = socket(AF_INET, SOCK_STREAM, 0);//ipv4, 流传输，tcp

    //2.绑定地址
    //写入地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;//ipv4
    //IP地址，转换和赋值
    inet_pton(AF_INET, IP, &addr.sin_addr.s_addr);
    addr.sin_port = htons(PROT);

    bind(fd_lis, (struct sockaddr*) &addr, sizeof(addr));

    //3.监听地址
    listen(fd_lis, 8);

    //4.接受连接
    struct sockaddr_in clientaddr;
    socklen_t len = 0;
    int fd_cli = accept(fd_lis, (struct sockaddr*)&clientaddr, &len);
    

    //输出客户端的信息
    char clientIP[16];
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    unsigned prot_cli = ntohs(clientaddr.sin_port);
    printf("客户端已连接。\n客户端ip地址: %s\n客户端端口地址:%d\n", clientIP, prot_cli);

    //5.通信
    //获取数据
    char recvbuf[1024] = {};
    char sendbuf[1024] = {};
    while(1){
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));

        int buflen = read(fd_cli, recvbuf, sizeof(recvbuf));
        if(buflen == -1){
            perror("read");
            return -1;
        }else if(buflen > 0){
            printf("recv client data: %s\n", recvbuf);
        }else if(buflen == 0){
            //客户端断开连接
            printf("client closed...\n");
            break;
        }
        //发送数据
        strcpy(sendbuf, recvbuf);//环回
        write(fd_cli, sendbuf, strlen(sendbuf));
    }

    //6.关闭文件描述符
    close(fd_cli);
    close(fd_lis);


    return 0;

}