#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H
#include <pthread.h>

/*
    采用RAII原则封装mutex的创建，销毁，加锁，解锁这四个操作
    不手动调用加锁和解锁操作，一切交由对象的构造和析构来完成
    TODO 如何判断对象的析构时刻是需要的时刻
*/
class MutexLock
{
private:
    pthread_mutex_t mutex;

public:
    MutexLock();
    ~MutexLock();
};

MutexLock::MutexLock(): mutex(PTHREAD_MUTEX_INITIALIZER)
{
    pthread_mutex_lock(&mutex);
}

MutexLock::~MutexLock() // 似乎有些问题: 当unlock以后如果被其他线程获得互斥锁，此时销毁，会产生问题
{
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
}

#endif