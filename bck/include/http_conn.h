#ifndef HTTPCONNECT
#define HTTPCONNECT


#include <sys/epoll.h>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <iostream>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/uio.h>


class http_conn
{
public:


    //HTTP请求方法，目前只支持GET
    enum METHOD{GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};

    /*解析客户端请求时，主状态机的状态
    CHECK_STATE_REQUESTLITE:正在分析请求行
    CHECK_STATE_HEADER:正在分析头部字段
    CHECK_STATE_CONTENT:正在分析请求体 */
    enum CHECK_STATE{CHECK_STATE_REQUESTLITE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT};

    /*服务器处理HTTP请求的可能结果，报文解析的结果
    NO_REQUEST:请求不完整，需要继续读取用户数据
    GET_REQUEST:表示获得了一个完成的客户请求
    BAD_REQUEST:表示客户请求语法错误
    NO_RESOURCE:表示服务器没有资源
    FORBIDDEN_REQUESET:表示客户端没有权限访问资源
    FILE_REQUEST:文件请求，获取文件成功
    INTERNAL_ERROR:服务器内部错误
    CLOSED_CONNECTION:客户端已关闭连接*/
    enum HTTP_CODE {NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};


    /*从状态机的三种可能状态
    1.读取到一个完整的行 2.行出错 3.行数据不完整
    */
   enum LINE_STATUS{LINE_OK = 0, LINE_BAD, LINE_OPEN};


    static int m_epollfd;//所有socket上的事件都被注册到一个epoll对象内
    static int m_user_count;//用户计数
    
    static const int FILENAME_LEN = 200;        // 文件名的最大长度
    static const int READ_BUFFER_SIZE = 2048; //读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 2048; //写缓冲区大小

    http_conn();
    ~http_conn();
    void process();//处理客户端请求
    void init(int sockfd, const sockaddr_in & addr);//初始化新的连接
    void close_conn();//关闭连接
    bool read();//读数据
    bool write();//写数据



    

private:

    void init();//初始化解析相关成员
    HTTP_CODE process_read();// 解析HTTP请求
    HTTP_CODE parse_request_line(char* text);//解析请求首行
    HTTP_CODE parse_request_header(char* text);//解析请求头
    HTTP_CODE parse_request_content(char* text);//解析请求体
    LINE_STATUS parse_line();//解析单行
    HTTP_CODE do_request();// 执行请求
    void unmap();// 释放内存映射


    bool process_write(HTTP_CODE ret);// 生成响应


    //写报文要用到的函数
    bool add_response(const char* format, ...);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_length);
    bool add_content(const char* content);
    bool add_content_length(int content_len);
    bool add_content_type();
    bool add_linger();
    bool add_blank_line();

    char* get_line() { return m_read_buf + m_start_line;}


    int m_sockfd;                           //该对象连接的socket
    sockaddr_in m_address;                  //该对象的地址

    char m_read_buf[READ_BUFFER_SIZE];      //读缓冲区
    int m_read_index;                       //标识读缓冲区中以及读入客户端数据的最后一个字节的下一内容
    char* m_url;                            //请求目标文件的文件名
    char* m_version;                        //请求协议版本，目前只支持HTTP1.1
    METHOD m_method;                        //请求方法
    char* m_host;                           // 请求主机名
    bool m_linger;                          //HTTP请求是否保持连接
    long m_content_length;                  //请求体长度
    
    
    char m_write_buf[WRITE_BUFFER_SIZE];    //写缓冲区
    int m_write_index;                      //写缓冲区指针
    char m_real_file[200];                  //客户端请求资源的完整路径，内容等于doc_root + m_url, doc_root是网站根目录
    struct stat m_file_stat;                //文件状态
    char* m_file_address;                   // 文件映射地址
    struct iovec m_iv[2];                   // 我们将采用writev来执行写操作，所以定义下面两个成员，其中m_iv_count表示被写内存块的数量。
    int m_iv_count;

    int m_bytes_to_send; //需要发送的报文长度
    int m_bytes_have_send; //已经发送的报文长度


    int m_checked_index; //当前在分析的字符在读缓冲区的位置
    int m_start_line; //当前在解析的行的起始位置

    CHECK_STATE m_check_state; //主状态机所处的状态
};






void modfd(int epollfd, int fd, int ev);
void addfd(int epollfd, int fd, bool oneshot, bool et);

void rmfd(int epollfd, int fd);
#endif