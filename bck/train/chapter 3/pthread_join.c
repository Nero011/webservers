#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
char* str = "子线程退出";
int a  = 10;

void* callback(void* arg){

    printf("str的地址: %p\n", str);
    // printf("a的地址: %p\n", &a);
    printf("%s\n", str);
    pthread_exit((void*) str);
}


int main(){
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, callback, NULL);
    if (ret != 0){
        char* str = strerror(ret);
        printf("%s\n", str);
    }


    //主线程回收线程
    //二级指针，获取线程退出信息
    char* retstr;
    printf("retstr的地址: %p\n", retstr);
    ret = pthread_join(tid, (void**)&retstr);
    printf("retstr的地址: %p\n", retstr);
    printf("retstr存放的东西: %s\n", (char*)retstr);
    printf("retstr存放的东西: %s\n", retstr);
    if(ret == 0) {
        printf("回收子线程\n");
        // printf("返回数据：%s\n", *retstr);
    }

    //主线程退出，主线程退出也不会影响其他线程
    pthread_exit(NULL);

    return 0;
}