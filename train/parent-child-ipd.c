/*
实现类似 ps aux | grep 的效果
子进程执行 ps aux ，通过管道把数据发送到父进程进行提取

*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <wait.h>

int main(){
    int ret;
    
    //创建管道
    int pipefd[2];
    ret = pipe(pipefd);
    if (ret < 0){
        perror("pipe");
        return -1;
    }

    //创建子进程
    pid_t pid = fork();
    if(pid > 0){
        //父进程
        close(pipefd[1]);//关闭写端fd
        char buf[1024] = {};
        int len = -1;
        while(len = read(pipefd[0], buf, sizeof(buf)-1)){
            bzero(buf, 1024);
            printf("%s", buf);
        }
        wait(NULL);

    }else if(pid == 0){
        //子进程
        close(pipefd[0]);//关闭读端fd
        dup2(pipefd[1], STDOUT_FILENO);//重定向标准输出
        execlp("ps", "ps", "aux", NULL);//调用ps aux
        //这两句在exec调用成功后不会执行
        perror("exec");
        return 0;
    }else{
        perror("fork");
        return -1;
    }

    return 0;

}