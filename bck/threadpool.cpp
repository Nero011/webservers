#include "threadpool.h"


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

        if(pthread_detach(m_threads+i) != 0){
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
    return true;

}

template<typename T>
void* threadpool<T>::work(void* arg){

    threadpool* pool = (threadpool*)arg;//this指针
    pool->run();//这里需要封装多一层吗？

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

        request->process();//处理任务
    }


}