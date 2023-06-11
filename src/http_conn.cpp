#include "http_conn.h"


int http_conn::m_epollfd;
int http_conn::m_user_count;
const int http_conn::READ_BUFFER_SIZE;
const int http_conn::WRITE_BUFFER_SIZE;


// 定义HTTP响应的一些状态信息
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to satisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file from this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the requested file.\n";


// 网站的根目录
const char* doc_root = "/home/thatchlobe/cpp/webservers/resources";

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
    m_checked_index = 0;//初始化当前解析到字符的位置
    m_start_line = 0;
    m_read_index = 0;

    m_method = GET;
    m_url = nullptr;
    m_version = nullptr;
    m_linger = false;
    m_content_length = 0;
    m_host = 0;
    m_start_line = 0;
    m_write_index = 0;
    


    memset(m_read_buf, 0, READ_BUFFER_SIZE);
    memset(m_write_buf, 0, WRITE_BUFFER_SIZE);
    memset(m_real_file, 0, FILENAME_LEN);
    


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
    int bytes_send_ret = 0; // 本轮发送的字节数
    int bytes_have_send = 0;// 已经发送的字节数
    int bytes_to_send = m_write_index; //将要发送的字节数 m_write_index: 写缓冲区中将要发送的字节数

    if(bytes_to_send == 0){ // 没有要发送的数据，本次响应结束
        modfd(m_epollfd, m_sockfd, EPOLLIN);
        init();
        return true;
    }

    while(true){

        //分散写，用writev，因为要从两片内存中读取数据，一片是自己拼接好的报文头，另一部分是请求的资源
        bytes_send_ret = writev(m_sockfd, m_iv, m_iv_count); // 本轮发送的字节数
        if(bytes_send_ret <= -1){ // 本轮发送的字节数
            // 如果写缓冲区没有空间，则等待下一轮EPOLLOUT事件
            // 虽然在这期间，服务器无法接收客户端的下一个请求，但可以保证连接的完整性
            if(errno == EAGAIN){
                modfd(m_epollfd, m_sockfd, EPOLLOUT);
                return true;
            }

            unmap();
            return false;
        }

        bytes_have_send += bytes_send_ret;
        bytes_to_send -= bytes_send_ret;
        if(bytes_to_send <= bytes_have_send){
            std::cout << "bytes_to_sent <= bytes_have_send, 发送完毕" << std::endl;
            std::cout << "b2s = " << bytes_to_send << std::endl;
            std::cout << "bhs = " << bytes_have_send << std::endl;
            // 发送HTTP响应成功，根据HTTP请求中的Connect字段决定是否断开连接
            unmap();
            if(m_linger){
                init();
                modfd(m_epollfd, m_sockfd, EPOLLIN);
                return true;
            }else{
                modfd(m_epollfd, m_sockfd, EPOLLIN);
                return false;
            }
        }


    }
    return true;
}



// 响应相关函数
bool http_conn::add_response(const char* format, ...){
    if(m_write_index >= WRITE_BUFFER_SIZE){
        return false;
    }

    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_write_buf + m_write_index, WRITE_BUFFER_SIZE - 1 - m_write_index, format, arg_list);
    if(len >= (WRITE_BUFFER_SIZE - 1 - m_write_index)){
        return false;
    }

    m_write_index += len;
    va_end(arg_list);
    return true;
}

// 增加请求行
bool http_conn::add_status_line(int status, const char* title){
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}

// 增加请求头
bool http_conn::add_headers(int content_len){
    add_content_length(content_len);
    add_content_type();
    add_linger();
    add_blank_line();
    return true;
}

bool http_conn::add_content_length(int content_len){
    return add_response("Content-Length: %d\r\n", content_len);
}

// 请求类型
bool http_conn::add_content_type(){
    return add_response("Content-Type: %d\r\n", "text/html");
}

bool http_conn::add_linger(){
    return add_response("Connect:%s\r\n", (m_linger == true) ? "keep-alive" : "close");
}

bool http_conn::add_blank_line(){
    return add_response("%s", "\r\n");
}

bool http_conn::add_content(const char* content){
    return add_response( "%s", content );
}

/**
* @brief          : 获得一个完整正确的HTTP请求后，先分析目标文件的属性，如果目标文件存在，
                    对所有用户可读，且不是目录，则使用mmap映射到内存地址m_file_address处，并告诉主调函数获取文件成功
* @param          : 
* @retval         : HTTP请求结果
*/
http_conn::HTTP_CODE http_conn::do_request(){
    strcpy(m_real_file, doc_root);
    int len = strlen(doc_root);
    strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);

    // 获取m_real_file文件相关的状态信息，-1失败，0成功
    if(stat(m_real_file, &m_file_stat) < 0){
        return NO_RESOURCE;
    }

    // 判断访问权限
    if(!(m_file_stat.st_mode) & S_IROTH){
        return FORBIDDEN_REQUEST;
    }

    // 判断是否为目录文件
    if(S_ISDIR(m_file_stat.st_mode)){
        return BAD_REQUEST;
    }

    // 以只读方式打开文件
    int fd = open(m_real_file, O_RDONLY);

    // 创建内存映射
    m_file_address = (char*)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;

}

// 对文件映射内存进行释放
void http_conn::unmap(){
    if(m_file_address){
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = nullptr;
    }
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
        // std::cout << text;

        m_start_line = m_checked_index; //更新行开始的位置为当前检查位置
        // printf("got 1 http line: %s\n", text);

        switch(m_check_state){ //主状态机
            case CHECK_STATE_REQUESTLITE:
            {
                ret = parse_request_line(text);
                // printf("method = %d, version = %s, url = %s\n", m_method, m_version, m_url);
                // std::cout << ret << std::endl;
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
* @brief          : 生成相应
* @param          : HTTP报文解析的结果
* @retval         : 成功：true; 失败：false
*/
bool http_conn::process_write(HTTP_CODE ret){
    switch (ret)
    {
    case INTERNAL_ERROR: //服务器内部错误
        add_status_line(500, error_500_title);
        add_headers(strlen(error_500_form));
        if(!(add_content(error_500_form))){
            return false;
        }
        break;
    
    case BAD_REQUEST: // 请求错误
        add_status_line(400, error_400_title);
        add_headers(strlen(error_400_form));
        if(!add_content(error_400_form)){
            return false;
        }
        break;

    case NO_RESOURCE: // 没有资源
        add_status_line(404, error_404_title);
        add_headers(strlen(error_404_form));
        if(!add_content(error_404_form)){
            return false;
        }
        break;


    case FORBIDDEN_REQUEST: // 没有权限，禁止访问
        add_status_line(403, error_403_title);
        add_headers(strlen(error_403_form));
        if(!add_content(error_403_form)){
            return false;
        }
        break;

    case FILE_REQUEST: // 文件请求
        add_status_line(200, ok_200_title);
        add_headers(m_file_stat.st_size);
        m_iv[ 0 ].iov_base = m_write_buf;
        m_iv[ 0 ].iov_len = m_write_index;
        m_iv[ 1 ].iov_base = m_file_address;
        m_iv[ 1 ].iov_len = m_file_stat.st_size;
        m_iv_count = 2;
        return true;

    default:
        break;
    }

    m_iv[ 0 ].iov_base = m_write_buf;
    m_iv[ 0 ].iov_len = m_write_index;
    m_iv_count = 1;
    return true;
}

/**
* @brief          : 解析HTTP请求行数据
* @param          : 一行字符串
* @retval         : 解析状态
*/
http_conn::HTTP_CODE http_conn::parse_request_line(char* text){
    printf("checking request line\n");
    m_url = strpbrk(text, " \t");//第一个空格的位置
    if (!m_url) { 
        return BAD_REQUEST;
    }
    *m_url++ = '\0';// 将空格位置变成字符串结束符，那么text就只包含空格前的内容了

    char* method = text;
    if(strcasecmp(method, "GET") == 0){ // 请求方法是GET
        m_method = GET;
    }else{
        return BAD_REQUEST; //暂时只支持GET
    }

    m_version = strpbrk(m_url, " \t");
    if(!m_version){
        return BAD_REQUEST;
    }
    *m_version++ = '\0';
    if(strcasecmp(m_version, "HTTP/1.1") != 0){
        return BAD_REQUEST;// 目前只支持HTTP1.1
    }

    if(strncasecmp(m_url, "http://", 7) == 0){//有时候会出现请求url是http://的情况
        m_url += 7;//把http://抛弃
        m_url = strchr(m_url, '/');//找到真正的url位置，忽略中间的服务器地址
        
    }
    if(!m_url || m_url[0] != '/'){
        return BAD_REQUEST;
    }

    m_check_state = CHECK_STATE_HEADER;//主状态机变成检查请求头


    
    return NO_REQUEST;
}
/**
* @brief          : 解析请求头
* @param          : 读到的行
* @retval         : 解析结果
*/
http_conn::HTTP_CODE http_conn::parse_request_header(char* text){
    if(text[0] == '\0'){ //传入为空，证明请求头已经解析完成
        //如果有HTTP请求体，则还要读取长度为m_content_length字节的请求体
        //状态机转换到CHECK_STATE_CONTENT状态
        if(m_content_length != 0){
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        // 否则则说明已经得到一个完整的请求
        return GET_REQUEST;
    }else if(strncasecmp(text, "Connection:", 11) == 0){
        //处理connection字段
        text += 11;
        text += strspn(text, " \t");//跳过空格，检查connection字段内容
        if(strcasecmp(text, "keep-alive") == 0){
            m_linger = true;
        }

    }else if(strncasecmp(text, "Content-Length:", 15) == 0){
        // 处理content-length字段
        text += 15;
        text += strspn(text, " \t");
        m_content_length = atol(text);
    }else if(strncasecmp(text, "Host:", 5) == 5){
        //处理Host字段
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    }else{
        printf("未完善的解析服务\n");
    }

    return NO_REQUEST;

}

//没有真正解析HTTP请求体
http_conn::HTTP_CODE http_conn::parse_request_content(char* text){
    printf("解析请求体\n");
    if ( m_read_index >= ( m_content_length + m_checked_index ) )
    {
        text[ m_content_length ] = '\0';
        return GET_REQUEST;
    }
    return NO_REQUEST;

}

/**
* @brief          : 处理单行数据，判断数据是\r\n
* @param          : 
* @retval         : 
*/
http_conn::LINE_STATUS http_conn::parse_line(){
    char temp;
    // std::cout << "处理单行" << std::endl;

    for(; m_checked_index < m_read_index; m_checked_index++){
        temp = m_read_buf[m_checked_index];
        // std::cout << "temp = " << temp << std::endl;
        // printf("m_check_index = %d, m_read_index = %d\n", m_checked_index, m_read_index);
        if(temp == '\r'){
            // std::cout << "\r" << std::endl;
            if((m_checked_index+1) == m_read_index){
                return LINE_OPEN; // 行未完整
            }else if(m_read_buf[m_checked_index+1] == '\n'){ //读到/r/n
                // 将/r/n替换为/0, 也就是字符串结束符，以方便字符串提取
                m_read_buf[m_checked_index++] = '\0';
                m_read_buf[m_checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }else if(temp == '\n'){ //有可能出现一行分开读的情况，恰好/r /n分开，所以检测到/n时也识别上一个内容，看看是否有一行数据
            // std::cout << "\n" << std::endl;
            if((m_checked_index > 1) && (m_read_buf[m_checked_index-1] == '\r')){
                m_read_buf[m_checked_index-1] = '\0';
                m_read_buf[m_checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }

    return LINE_OPEN;
}

/**
* @brief          : 由线程池中的工作线程调用，处理HTTP请求的入口函数，解析HTTP请求并生成响应
* @param          : 
* @retval         : 
*/
void http_conn::process(){
    printf("process http\n");


    // 解析HTTP请求
    HTTP_CODE read_ret = process_read();
    if(read_ret == NO_REQUEST){//请求不完整，继续监听
        modfd(m_epollfd, m_sockfd, EPOLLIN);
        return;
    }
    printf("完成HTTP解析，请求为%s\n", (read_ret == FILE_REQUEST) ? "FILE_REQUEST" : "未知请求");

    //生成响应
    bool write_ret = process_write(read_ret);
    if(!write_ret){
        close_conn();
    }
    modfd(m_epollfd, m_sockfd, EPOLLOUT);

}