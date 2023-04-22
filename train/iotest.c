#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int i){
    int fd1 = open("otest.txt", O_RDONLY);
    int fd2 = open("itest.txt", O_WRONLY);
    // chmod("itest.txt",0777);
    if(fd1 == -1) {
        perror("open otest.txt");
        return 0;
    }
    if(fd2 == -1) {
        perror("open itest.txt");
        return 0;
    }

    char readarr[1024] = {};
    // int count = 1024;

    int size = read(fd1, readarr, 200);
    printf("read %d byte\n", size);
    printf("%s\n",readarr);
    if(size>0){
        int wsize = write(fd2, readarr, size);
        printf("write %d byte\n", wsize);
    }

    // lseek(fd2, 0, SEEK_END);
    // write(fd2, readarr, size);

    close(fd1);
    close(fd2);
    return 0;
}