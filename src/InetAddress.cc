#include "InetAddress.h"

#include <cstring>

InetAddress::InetAddress(uint16_t port, std::string ip) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr =
        inet_addr(ip.c_str());  // 将string转换为const char*, 然后初始化地址
}

// 返回ip地址
std::string InetAddress::toIp() const {
    char buf[64];
    inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, sizeof(buf));
    return buf;
}

// 返回IP：PORT
std::string InetAddress::toIpPort() const {
    char ip[64];
    inet_ntop(AF_INET, &addr_.sin_addr.s_addr, ip, sizeof(ip));
    uint16_t port = ntohs(addr_.sin_port);
    size_t end = strlen(ip);
    sprintf(ip + end, ":%u", port);
    return ip;
}

// 返回端口号
uint16_t InetAddress::toPort() const {
    uint16_t port = ntohs(addr_.sin_port);
    return port;
}
