//线程池

#ifndef THREADPOOL_H
#define THREADPOOL_H


#include <pthread.h>
#include <list>
#include "locker.h"

//线程池类，定义为模板，为了代码复用
//T为任务类
template<typename T>
class threadpool{
private:
    int m_thread_num;//线程数量
    pthread_t* m_threads//线程池数组，大小为m_thread_num;
    int m_max_requests;//请求队列最大请求数量
    std::list<T*> m_workqueue;//请求队列
    locker m_queuelocker;//互斥锁
    sem m_queuestat;//信号量，判断是否有任务需要处理
    bool m_stop;//是否结束线程
public:
    threadpool();
    ~threadpool();
};





#endif