#include "EventLoop.h"
#include "net.h"
#include <iostream>
#include <arpa/inet.h>

using namespace std;

int main(){
    int lfd = openServer("127.0.0.1", 10080);

    EventLoop ev(lfd);
    ev.start();

    return 0;
}