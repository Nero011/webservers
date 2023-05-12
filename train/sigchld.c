//用SIGCHLD解决僵尸进程问题


#include <stdio.h>
#include <wait.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

void zombiepid(){
    while (1){    
        int pid = waitpid(0, NULL, WNOHANG);
        if(pid > 0){
            printf("recoery %d\n", pid);
        }else if(pid == 0){
            break;
        }else if(pid == -1){
            break;
        }

    }
    
}

int main(){
    //先设置SIGCHLD阻塞，避免没来得及注册捕捉信号，子进程就已经退出
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // 创建20个子进程
    pid_t pid;
    for(int i = 0; i < 20; i++){
        pid = fork();
        if(pid == 0){
            break;
        }
    }

    if(pid > 0){
        // 父进程
        struct sigaction act;
        act.sa_handler = zombiepid;
        sigemptyset(&act.sa_mask);
        // sigdelset(SIGCHLD, &act.sa_mask);
        act.sa_flags = 0;
        
        
        sigaction(SIGCHLD, &act, NULL);
        //注册信号捕捉后，取消信号阻塞
        sigprocmask(SIG_UNBLOCK, &set, NULL);
        while(1){
            //捕捉SIGCHLD信号，回收子进程
            printf("parent process pid: %d\n", getpid());
            sleep(2);
        }

        
    }else if(pid == 0){
        // 子进程
        printf("child pid: %d\n", getpid());
    }

    return 0;

}