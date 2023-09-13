#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>

class InetAddress {
private:
    sockaddr_in addr_;

public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    // 用于获得已有的地址
    explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}
    ~InetAddress();

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    const sockaddr_in* getSockAddr() const { return &addr_; }
    void setSockAddr(const sockaddr_in addr) { addr_ = addr; }
};
