// 编写一个程序，把所有常规信号(1-31)的未决状态打印到屏幕
// 设置某些信号是阻塞的，通过键盘产生这些信号

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

int main(){
	
	//设置2号信号和3号信号阻塞 ctrl+c ctrl+\
	
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGQUIT);

	//修改内核中的阻塞信号集
	sigprocmask(SIG_BLOCK, &set, NULL);
	
	while(1){
		//获取当前未决信号集的数据
		sigset_t pending_set;
		sigemptyset(&pending_set);
		sigpending(&pending_set);

		for(int i = 1; i <=32; i++){
			if(sigismember(&pending_set, i) == 1) printf("1 ");
			else if(sigismember(&pending_set, i) == 0) printf("0 ");

		}
		printf("\n");
	}

	return 0;
}

