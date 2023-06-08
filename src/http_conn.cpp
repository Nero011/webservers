#include "http_conn.h"


int http_conn::m_epollfd;
int http_conn::m_user_count;
const int http_conn::READ_BUFFER_SIZE;
const int http_conn::WRITE_BUFFER_SIZE;

http_conn::http_conn(){

}

http_conn::~http_conn(){

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
void addfd(int epollfd, int fd, bool oneshot, bool et){
    epoll_event event;
    event.data.fd = fd;
    if(et){
        event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;//ET触发, 连接断开
    }
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
    addfd(m_epollfd, m_sockfd, true, true);
    m_user_count++;
}

/**
* @brief          : 初始化解析报文相关成员变量
* @param          : 
* @retval         : 
*/
void http_conn::init(){
    m_check_state = CHECK_STATE_REQUESTLITE; //初始化状态为解析请求行
    m_checkde_index = 0;//初始化当前解析到字符的位置
    m_start_line = 0;
    m_read_index = 0;

    m_method = GET;
    m_url = nullptr;
    m_version = nullptr;
    m_linger = false;

    memset(m_read_buf, 0, READ_BUFFER_SIZE);

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
    printf("Client down.\n");
}

/**
* @brief          : 一次性读完所有数据，非阻塞
* @param          : 
* @retval         : 成功返回true, 失败返回flase
*/
bool http_conn::read(){
    
    if(m_read_index >= READ_BUFFER_SIZE){ //当读到数据超出缓冲区大小
        return false;
    }

    // 读取到的字节
    int bytes_read = 0;
    while(true){
        bytes_read = recv(m_sockfd, m_read_buf + m_read_index, READ_BUFFER_SIZE - m_read_index, 0);
        if(bytes_read == -1){
            if(errno == EAGAIN || errno == EWOULDBLOCK){ //没有数据了
                break;
            }
            return false;
        }else if(bytes_read == 0){//对方关闭连接
            return false;
        }
        m_read_index += bytes_read;
    }
    printf("%s\n", m_read_buf);

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

/**
* @brief          : 报文解析完成后，执行请求
* @param          : 
* @retval         : HTTP请求结果
*/
http_conn::HTTP_CODE http_conn::do_request(){

}


/**
* @brief          : 解析报文，主状态机
* @param          : 
* @retval         : 报文解析状态
*/
http_conn::HTTP_CODE http_conn::process_read(){

    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;

    char *text = nullptr;

    while( ((m_check_state == CHECK_STATE_CONTENT) && (line_status == LINE_OK)) || // 当前解析请求体，而且上面已经读完
        ((line_status = parse_line()) == LINE_OK)){ //解析完一行
        
        //获取一行数据
        text = get_line();

        m_start_line = m_checkde_index; //更新行开始的位置为当前检查位置
        printf("got 1 http line: %s\n", text);

        switch(m_check_state){ //主状态机
            case CHECK_STATE_REQUESTLITE:
            {
                ret = parse_request_line(text);
                if(ret == BAD_REQUEST){
                    return BAD_REQUEST;
                }
            }

            case CHECK_STATE_HEADER:
            {
                ret = parse_request_header(text);
                if(ret == BAD_REQUEST){
                    return BAD_REQUEST;
                } else if(ret == GET_REQUEST){ //获取到完整的请求
                    return do_request();// 执行请求
                }
            }

            case CHECK_STATE_CONTENT:
            {
                ret = parse_request_content(text);                
                if(ret == GET_REQUEST){
                    return do_request();
                }
                line_status = LINE_OPEN;
                break;
            }

            default:
            {
                return INTERNAL_ERROR;
            }
        }
        return NO_REQUEST;
    }


    return NO_REQUEST;
}

/**
* @brief          : 解析HTTP请求行数据
* @param          : 一行字符串
* @retval         : 解析状态
*/
http_conn::HTTP_CODE http_conn::parse_request_line(char* text){
    m_url = strpbrk(text, " \t");


}
http_conn::HTTP_CODE http_conn::parse_request_header(char* text){

}
http_conn::HTTP_CODE http_conn::parse_request_content(char* text){

}

/**
* @brief          : 处理单行数据，判断数据是\r\n
* @param          : 
* @retval         : 
*/
http_conn::LINE_STATUS http_conn::parse_line(){
    char temp;

    for(; m_checkde_index < m_read_index; m_checkde_index++){
        temp = m_read_buf[m_checkde_index];
        if(temp == '/r'){
            if((m_checkde_index+1) == m_read_index){
                return LINE_OPEN; // 行未完整
            }else if(m_read_buf[m_checkde_index+1] == '\n'){ //读到/r/n
                // 将/r/n替换为/0, 也就是字符串结束符，以方便字符串提取
                m_read_buf[m_checkde_index++] = '/0';
                m_read_buf[m_checkde_index++] = '/0';
                return LINE_OK;
            }
            return LINE_BAD;
        }else if(temp == '/n'){ //有可能出现一行分开读的情况，恰好/r /n分开，所以检测到/n时也识别上一个内容，看看是否有一行数据
            if((m_checkde_index > 1) && (m_read_buf[m_checkde_index-1] == '/r')){
                m_read_buf[m_checkde_index-1] = '/0';
                m_read_buf[m_checkde_index++] = '/0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        return LINE_OPEN;
    }


    return LINE_OK;
}

/**
* @brief          : 由线程池中的工作线程调用，处理HTTP请求的入口函数，解析HTTP请求并生成响应
* @param          : 
* @retval         : 
*/
void http_conn::process(){
    HTTP_CODE read_ret = process_read();
    if(read_ret == NO_RESOURCE){//请求不完整，继续监听
        modfd(m_epollfd, m_sockfd, EPOLLIN);
        return;
    }



}