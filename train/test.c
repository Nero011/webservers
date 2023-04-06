#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int i){
    int fd1 = open("otest.txt", O_RDONLY);
    int fd2 = open("itest.txt", O_WRONLY);
    if(fd1 == -1) {
        perror("open otest.txt");
        return 0;
    }
    if(fd2 == -1) {
        perror("open itest.txt");
        return 0;
    }

    char* readarr[1024] = {};
    int count = 1024;

    read(fd1, readarr, count);
    write(fd2, readarr, count);
    return 0;
}