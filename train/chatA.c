#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int main(){
    int ret = access("fifo1", F_OK);
    if(ret == -1){
        printf("fifo is not exist\n");
        ret = mkfifo("fifo1", 0664);
        if(ret == -1){
            perror("mkfifo");
            exit(0);
        }
    }
    ret = access("fifo2", F_OK);
    if(ret == -1){
        printf("fifo is not exist\n");
        ret = mkfifo("fifo2", 0664);
        if(ret == -1){
            perror("mkfifo");
            exit(0);
        }
    }

    int fdw = open("fifo1", O_WRONLY);
    if(fdw<0){
        perror("open");
        exit(0);
    }
    printf("open fifo succeed, wait for writing...\n");

    int fdr = open("fifo2", O_RDONLY);
    if(fdr<0){
        perror("open");
        exit(0);
    }
    printf("open fifo succeed, wait for reading...\n");    


    pid_t pid = fork();
    char buf[128];

    while(1){
        if(pid > 1){
            memset(buf, 0, 128);
            fgets(buf, 128, stdin);

            ret = write(fdw, buf, strlen(buf));
            if(ret == -1){
                perror("write");
                break;
            }
        }
        if(pid == 0){
            memset(buf, 0, 128);
            ret = read(fdr, buf, 128);
            if(ret <= 0){
                printf("READ EOF!\n");
                perror(" ");
                break;
            }
            printf("buf: %s", buf);
        }

    }

    close(fdr);
    close(fdw);

    return 0;

}