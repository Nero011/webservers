#ifndef HTTPCONNECT
#define HTTPCONNECT
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>

class http_conn
{
private:
    int m_sockfd;//该对象连接的socket
    sockaddr_in m_address;//该对象的地址
public:
    static int m_epollfd;//所有socket上的事件都被注册到一个epoll对象内
    static int m_user_count;//用户计数

    http_conn();
    ~http_conn();
    void process();//处理客户端请求
    void init(int sockfd, const sockaddr_in & addr);//初始化新的连接
    void close_conn();//关闭连接
    bool read();//读数据
    bool write();//写数据
};


void addfd(int epollfd, int fd, bool oneshot);

void rmfd(int epollfd, int fd);

void modfd(int epollfd, int fd, int ev);



#endif