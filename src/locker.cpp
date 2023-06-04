#include <locker.h>

locker::locker(){//初始化互斥锁
    if(pthread_mutex_init(&m_mutex, NULL) != 0) { //初始化，如果初始化失败，抛出异常
        throw std::exception();
    }
}

locker::~locker(){
    pthread_mutex_destroy(&m_mutex);
}

bool locker::lock(){//上锁，成功返回0，如果成功函数返回真
    return pthread_mutex_lock(&m_mutex) == 0;
}

bool locker::unlock(){
    return pthread_mutex_unlock(&m_mutex) == 0;
}

pthread_mutex_t* locker::get(){
    return &m_mutex;
}

cond::cond(){
    if(pthread_cond_init(&m_cond, NULL) != 0){
        throw std::exception();
    }
}

cond::~cond(){
    if(pthread_cond_destroy(&m_cond) != 0){
        throw std::exception();
    }
}

bool cond::wait(pthread_mutex_t* mutex){//等待
    return pthread_cond_wait(&m_cond, mutex) == 0;
}

bool cond::timewait(pthread_mutex_t* mutex, struct timespec t){//定时等待
    return pthread_cond_timedwait(&m_cond, mutex, &t) == 0;
}

bool cond::signal(){
    return pthread_cond_signal(&m_cond) == 0;
}

bool cond::broadcast(){
    return pthread_cond_broadcast(&m_cond) == 0;
}


sem::sem(){
    if(sem_init(&m_sem, 0, 0) != 0){
        throw std::exception();
    }
}
sem::sem(int num){
    if(sem_init(&m_sem, 0, num) != 0){
        throw std::exception();
    }
}

sem::~sem(){
    if(sem_destroy(&m_sem) == 0){
        throw std::exception();
    }
}

bool sem::wait(){
    return sem_wait(&m_sem) == 0;
}

bool sem::post(){
    return sem_post(&m_sem) == 0;
}