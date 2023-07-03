#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

int main(){

    //创建共享内存
    int shmid = shmget(100, 4096, IPC_CREAT|0664);

    //和当前进程关联
    void* ptr = shmat(shmid, NULL, 0);

    //写数据
    char* str = "hello shm";
    memcpy(ptr, str, strlen(str) + 1);

    printf("按任意键继续\n");
    getchar();

    //解除关联
    shmdt(ptr);

    //删除共享内存
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}