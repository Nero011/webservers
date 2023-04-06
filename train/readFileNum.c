/*
读取某个目录以及其子目录下所有普通文件的个数
利用目录遍历函数
*/

//开启宏
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

int getFileNum(const char* path);

int main(int argc, char* argv[]){
    
    // 输入判断
    if(argc < 2){
        printf("please enter dir + %s", argv[0]);
        return -1;
    }

    // char path[256] = "/home/thatchlobe/cpp/webservers";

    int num = getFileNum(argv[1]);
    // int num = getFileNum(path);
    printf("REGFILE nums is %d\n", num);

    return 0;

}

int getFileNum(const char* path){

    //普通文件总数
    int totel = 0;

    //打开目录
    DIR *dirp = opendir(path);
    if(dirp == NULL) {
        perror("opendir");
        // return 0;也行的
        exit(0);
    }

    //读目录信息
    struct dirent* dir;

    //当前目录未读到EOF时，进入循环
    //readir要放在循环内，不然指针不能推进
    while((dir = readdir(dirp)) != NULL){
        
        //排除当前目录和上一层目录
        char* name = dir->d_name;
        if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        //读到当前文件时目录时，进入下一层递归
        if(dir->d_type == DT_DIR){
            char dirpath[256];
            sprintf(dirpath,"%s/%s", path, name);
            totel += getFileNum(dirpath);
        }
    
        //读到普通文件时
        if(dir->d_type == DT_REG) totel++;
    }

    //关闭目录
    closedir(dirp);

    return totel;
}