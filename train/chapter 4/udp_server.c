#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

const int PORT = 8888;
char* IP = "192.168.88.129";


int main(){
    //1.创建socket
    int fd_s = socket(AF_INET, SOCK_DGRAM, 0);//ipv4, 包传输，udp

    //2.绑定
    struct sockaddr_in addr_s;
    addr_s.sin_family = AF_INET;
    addr_s.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &addr_s.sin_addr.s_addr);

    bind(fd_s, (struct sockaddr*)&addr_s, sizeof(addr_s));

    //3.接收数据
    struct sockaddr_in addr_c;
    int len = sizeof(addr_c);

    char recvbuf[1024] = {};

    while(1){
        memset(recvbuf, 0, 1024);
        recvfrom(fd_s, recvbuf, 1024, 0, (struct sockaddr*)&addr_c, &len);
        printf("recvbuf: %s\n", recvbuf);

        //4.发送数据
        sendto(fd_s, recvbuf, 1024, 0, (struct sockaddr*)&addr_c, sizeof(addr_c));
    }




    close(fd_s);
}