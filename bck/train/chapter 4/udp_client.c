#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

const int PORT = 8888;
char* IP = "192.168.88.129";


int main(){
    //1.创建socket
    int fd_c = socket(AF_INET, SOCK_DGRAM, 0);//ipv4, 包传输，udp


    while(1){

        //2.发送数据
        struct sockaddr_in addr_s;
        addr_s.sin_family = AF_INET;
        addr_s.sin_port = htons(PORT);
        inet_pton(AF_INET, IP, &addr_s.sin_addr.s_addr);
        char sendbuf[1024] = {};
        fgets(sendbuf, sizeof(sendbuf), stdin);
        sendto(fd_c, sendbuf, 1024, 0, (struct sockaddr*)&addr_s, sizeof(addr_s));


        //3.接收数据


        char recvbuf[1024] = {};
        recvfrom(fd_c, recvbuf, 1024, 0, NULL, NULL);
        printf("recvbuf: %s\n", recvbuf);
    }




    return 0;
}