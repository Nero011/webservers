//线程池
#ifndef LOCKER_H
#define LOCKER_H


#include <pthread.h>//线程
#include <exception>//异常
#include <semaphore.h>//信号量

//线程同步类

//互斥锁

class locker {
public:
    locker();
    ~locker();
    bool lock();//上锁
    bool unlock();//解锁
    pthread_mutex_t* get();//获取互斥锁本身
    

private:
    pthread_mutex_t m_mutex;//互斥锁
};



//条件变量

class cond{
private:
    pthread_cond_t m_cond;//条件变量
public:
    cond();
    ~cond();
    bool wait(pthread_mutex_t* mutex);
    bool timewait(pthread_mutex_t* mutex, struct timespec t);
    bool signal();
    bool broadcast();
};

//信号量
class sem{
private:
    sem_t m_sem;
    
public:
    sem();
    sem(int num);
    ~sem();


    bool wait();//等待信号量
    bool post();//增加信号量

};





#endif