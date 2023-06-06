//线程池

#ifndef THREADPOOL_H
#define THREADPOOL_H


#include <pthread.h>
#include <list>
#include <exception>
#include "locker.h"
#include <stdio.h>


//线程池类，定义为模板，为了代码复用
//T为任务类
template<typename T>
class threadpool{
private:
    int m_thread_num;//线程数量
    pthread_t* m_threads;//线程池数组，大小为m_thread_num;
    int m_max_requests;//请求队列最大请求数量
    std::list<T*> m_workqueue;//请求队列
    locker m_queuelocker;//互斥锁
    sem m_queuestat;//信号量，判断是否有任务需要处理
    bool m_stop;//是否结束线程

private:
    static void* work(void* arg);
    void run();

public:
    threadpool(int thread_num = 8, int max_requests = 1);
    ~threadpool();
    bool append(T* request);//增加任务
};



template<typename T>
threadpool<T>::threadpool(int thread_num, int max_requeues):
    m_thread_num(thread_num), m_max_requests(max_requeues),
    m_threads(NULL), m_stop(false)    
{
    if((thread_num <= 0) || (max_requeues <= 0)){//传递了错误的参数
        throw std::exception();
    }

    m_threads = new pthread_t[m_thread_num];//创建线程
    if(!m_threads){
        throw std::exception();
    }

    //创建thread_num个线程，并设置线程分离
    for(int i = 0;i < m_thread_num; i++){
        printf("create the %dth thread\n", i);
        if(pthread_create(m_threads+i, NULL, work, this) != 0){
            delete[] m_threads;
            throw std::exception();
        }

        if(pthread_detach(m_threads[i]) != 0){
            delete[] m_threads;
            throw std::exception();
        }
    }

}

template<typename T>
threadpool<T>::~threadpool(){
    delete[] m_threads;
    m_stop = true;
}

template<typename T>
bool threadpool<T>::append(T* request){
    m_queuelocker.lock();//获取互斥锁
    //请求队列大小超出请求队列最大大小
    if(m_workqueue.size() > m_max_requests){//>还是>=
        m_queuelocker.unlock();
        return false;
    }

    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    printf("新增线程任务\n");
    return true;

}





template<typename T>
void* threadpool<T>::work(void* arg){

    threadpool* pool = (threadpool*)arg;//this指针
    pool->run();//这里需要封装多一层吗？
    return pool;
}

template<typename T>
void threadpool<T>::run(){

    while(!m_stop){
        m_queuestat.wait();//有任务
        m_queuelocker.lock();
        if(m_workqueue.empty()){//请求队列为空，下一轮循环
            m_queuelocker.unlock();
            continue;
        }

        //获取任务
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();

        if(!request){//没任务
            continue;
        }
        printf("线程获取任务\n");
        request->process();//处理任务
    }


}


#endif