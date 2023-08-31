#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
using namespace std;

int openServer(char* ip, int port){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(atoi(ip));    

    bind(fd, (sockaddr*)&addr, sizeof(addr));

    listen(fd, -1);

    return fd;
}