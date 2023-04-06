// 模拟实现ls-l功能
// -rw-rw-r-- 1 thatchlobe thatchlobe 121  4月  2 20:54 src/otest.txt
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

int main(int argc, char * argv[]){
    
    //程序运行提示
    if(argc < 2){
        printf("%s filename", argv[0]);
        return -1;
    }

    //用stat函数获取传入文件的信息
    struct stat st;
    int ret = stat(argv[1], &st);
    if(ret == -1){
        perror("stat:");
        return -1;
    }

    //获取文件的类型权限信息
    char type[11] = {};
    switch (st.st_mode & __S_IFMT)
    {
    case __S_IFBLK:
        type[0] = 'b';
        break;
    case __S_IFSOCK:
        type[0] = 's';
        break;
    case __S_IFLNK:
        type[0] = 'l';
        break;
    case __S_IFREG:
        type[0] = '-';
        break;
    case __S_IFDIR:
        type[0] = 'd';
        break;
    case __S_IFCHR:
        type[0] = 'c';
        break;
    case __S_IFIFO:
        type[0] = 'p';
        break;
    
    default:
        type[0] = '?';
        break;
    }
    type[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    type[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    type[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    type[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    type[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    type[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    type[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    type[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    type[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';

    //获取连接数
    long int link = st.st_nlink;

    //获取文件所有者
    char* fileUser = getpwuid(st.st_uid)->pw_name;
    
    //获取文件所在组
    char* fileGrp = getgrgid(st.st_gid)->gr_name;

    //获取文件大小
    long int fileSize = st.st_size;
    
    //获取修改时间
    char* time = ctime(&st.st_mtime);

    char buf[1024];
    sprintf(buf, "%s %ld %s %s %ld %s %s", type, link, fileUser, fileGrp, fileSize, time, argv[1]);

    printf("%s\n", buf);

    return 0;

}