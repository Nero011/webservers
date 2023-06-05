#include "http_conn.h"


int http_conn::m_epollfd;
int http_conn::m_user_count;

http_conn::http_conn(){

}

http_conn::~http_conn(){

}

/**
* @brief          : 由线程池中的工作线程调用，处理HTTP请求的入口函数，解析HTTP请求并生成响应
* @param          : 
* @retval         : 
*/
void http_conn::process(){

}

//设置文件描述符非阻塞
void setnonblock(int fd){
    int old_flag = fcntl(fd, F_GETFL);
    int flag = old_flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
}

/**
* @brief          : 添加文件描述符到epoll中
* @param          : epollfd：epoll的文件描述符; fd: 需要添加的文件描述符; oneshot；是否开启oneshot
* @retval         : 
*/
void addfd(int epollfd, int fd, bool oneshot){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;//LT触发, 连接断开

    if(oneshot){
        event.events |= EPOLLONESHOT;
    }

    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);

    //设置文件描述符非阻塞
    setnonblock(fd);
}

/**
* @brief          : 从epoll中删除文件描述符
* @param          : epollfd: epoll的文件描述符; fd: 需要删除的文件描述符
* @retval         : 
*/
void rmfd(int epollfd, int fd){
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

/**
* @brief          : 修改epoll中的文件描述符
* @param          : epollfd: epoll的文件描述符; fd: 需要修改的文件描述符; ev: 需要修改的事件
* @retval         : 
*/
void modfd(int epollfd, int fd, int ev){
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;//重置oneshot事件，确保下一次可读时会触发
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);

}

/**
* @brief          : 初始化新的客户连接
* @param          : sockfd: 新连接的文件描述符; addr: 新连接的地址信息
* @retval         : 
*/
void http_conn::init(int sockfd, const sockaddr_in & addr){
    m_sockfd = sockfd;
    m_address = addr;

    //端口复用
    int reuse = 1;
    setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    //添加进epoll
    addfd(m_epollfd, m_sockfd, true);
    m_user_count++;
}

/**
* @brief          : 关闭连接
* @param          : 
* @retval         : 
*/
void http_conn::close_conn(){
    if(m_sockfd != -1){
        rmfd(m_epollfd, m_sockfd);//从epoll中移除
        m_sockfd = -1;
        m_user_count--;
    }
}

/**
* @brief          : 一次性读完所有数据，非阻塞
* @param          : 
* @retval         : 成功返回true, 失败返回flase
*/
bool http_conn::read(){
    return true;
}
/**
* @brief          : 一次性写完所有数据，非阻塞
* @param          : 
* @retval         : 成功返回true, 失败返回flase
*/
bool http_conn::write(){
    return true;
}