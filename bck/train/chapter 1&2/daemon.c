/*
    使用守护进程，每个2s获取系统事件，并将这个时间写入磁盘文件
*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

void timesir(int num){
    printf("timesir\n");
    int fd = open("time.txt", O_RDWR | O_CREAT | O_APPEND
    , 0664);
    time_t rtime = time(NULL);
    struct tm *loc = localtime(&rtime);
    // char buf[1024];
    // sprintf(buf, "%d-%d-%d %d:%d:%d", loc->tm_year, loc->tm_mon, loc->tm_mday, loc->tm_hour, loc->tm_min, loc->tm_sec);
    char * str = asctime(loc);
    write(fd, str, strlen(str));
    close(fd);
    printf("%s\n", str);
    
}

int main(){
    //1.fork，并让父进程退出
    pid_t pid = fork();
    if(pid > 0){
        //父进程
        printf("父进程退出\n");
        exit(0);
    }
    //2.通过子进程创建新会话
    pid_t sid = setsid();

    //3.清除umask
    umask(022);
    //4.修改工作目录
    chdir("/home/thatchlobe");
    //5.关闭所有文件描述符


    //6.重定向文件描述符
    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    //7.核心逻辑


    //定时器
    struct itimerval timer;
    timer.it_interval.tv_sec = 2;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;

    //注册信号捕捉
    struct sigaction action;
    action.sa_handler = timesir;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    int ret = sigaction(SIGALRM, &action, NULL);
    //用sigaction
    // struct sigaction act;
    // act.sa_handler = timesir;//回调函数
    // sigemptyset(&act.sa_mask);//清空临时阻塞信号集
    // act.sa_flags = 0;//调用sa_handler

    // int ret = sigaction(SIGALRM, &act, NULL);
    if(ret == -1){
        perror("注册信号捕捉");
        return -1;
    }else{
        printf("等待信号中断\n");
    }
    //设置定时器
    ret = setitimer(ITIMER_REAL, &timer, NULL);
    if(ret == -1){
        perror("设置定时器");
        return -1;
    }else{
        printf("定时开启\n");
    }

    while (1)
    {
        printf("1\n");
        sleep(1);

    }
    
    return 0;
}