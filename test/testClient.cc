#include <iostream>

#include "mymuduo/InetAddress.h"
#include "mymuduo/Socket.h"

// g++ test/testClient.cc -o client.o -lmymuduo -lpthread -g

class Client {
private:
    Socket sock_;
    int connfd_;
    InetAddress addr_;

public:
    Client(InetAddress& targetAddr) : addr_(addr_), connfd_(-1), sock_(socket(AF_INET, SOCK_STREAM, 0)) {
        int fd = sock_.fd();
        if (fd < 0) {
            exit(0);
        }
    }
    ~Client() { sock_.shutdownWrite(); }

    void connect() { connfd_ = sock_.connect(addr_); }

    void write(std::string& s) { ::write(connfd_, &*s.begin(), s.size()); }
    void read(std::string& s) { ::read(connfd_, &*s.begin(), 1024); }
};

int main() {
    InetAddress target(8008);
    Client one(target);
    one.connect();

    std::string writebuf;
    std::string readbuf(1024, 0);
    while (1) {
        writebuf.clear();
        std::cin >> writebuf;
        one.write(writebuf);
        readbuf.clear();
        one.read(readbuf);
        std::cout << readbuf << std::endl;
    }
    return 0;
}