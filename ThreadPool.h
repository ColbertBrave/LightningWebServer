#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>

/*
    线程池类
    功能:
        创建不超过最大线程数量的线程;
        取出并处理请求;
        添加请求;
        关闭线程池;
*/
template<typename T>
class ThreadPool
{
private:
    std::list<T *>          Request_List;               // 思考Request_List和ThreadID_List选择这样的数据结构的原因
    std::vector<pthread_t>  ThreadID_List;      // pthread_t是线程的ID类型
    pthread_mutex_t         mutex;
    int Max_Requests;
    int Thread_Num;
    static const int Max_Thread_Num = 16;       // 作为特例，有序型的const静态数据成员可以在类体中用一常量值初始化
    bool Server_IsOn;        
    
    static bool worker();
    void run();

public:
    ThreadPool(int Thread_Num);
    ~ThreadPool();

    bool append(T *request);
    void shutDown();
}

#endif