#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>


int main(){

    pid_t pid = fork();

    if(pid>0){
        printf("this is parent process, pid = %d, ppid = %d\n", getpid(), getppid());
    }else if(pid == 0){
        printf("this is child process, pid = %d, ppid = %d\n", getpid(), getppid());
    }

    return 0;
}