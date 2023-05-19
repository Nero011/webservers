#include <stdio.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>

int main(){
    
    int shmid = shmget(100, 4096, IPC_EXCL);

    void* ptr = shmat(shmid, NULL, 0);


    printf("%s\n", (char*)ptr);

    printf("请按任意键继续\n");
    getchar();

    shmdt(ptr);

    shmctl(shmid, IPC_RMID, NULL);

    return 0;

}