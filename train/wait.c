#include <stdio.h>
#include <wait.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
    pid_t pid;
    
    for(int i = 0; i<5; i++){
        pid = fork();
        if(pid == 0){
            break;
        }
    }

    if(pid>0){
        //父进程
        while(1){
            printf("parent, pid = %d", getpid());
            int* status;
            int ret = wait(status);
            printf("pid = %d, status: %d\n", ret, *status);
        }
    }else if(pid == 0){
        //子进程
        while (1)
        {
            
        }
        
    }
}