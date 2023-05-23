//TCP客户端程序

#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

const int PROT_SER = 8888;
const char *IP_SER = "192.168.88.129";


int main(){

    //1.创建socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);//ipv4, 流传输，tcp协议



    //2.连接
    struct sockaddr_in addr_ser;//服务器的地址
    addr_ser.sin_family = AF_INET;
    addr_ser.sin_port = htons(PROT_SER);
    inet_pton(AF_INET, IP_SER, &addr_ser.sin_addr.s_addr);
    int ret = connect(fd, (struct sockaddr*)&addr_ser, sizeof(addr_ser));
    if(ret = 0) printf("Connected.\n");

    //3.通信
    char send_buf[1024];
    printf("Enter text: \n");
    scanf("%s", send_buf);
    write(fd, send_buf, strlen(send_buf));


    char recv_buf[1024];
    ret = read(fd, recv_buf, sizeof(recv_buf));
    if(ret > 0){
        printf("%s\n", recv_buf);
    }

    //5.关闭文件描述符
    close(fd);
    

    return 0;

}