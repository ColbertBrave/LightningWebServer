#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>

template<typename T>
class ThreadPool
{
private:
    std::list<T *> Request_List;               // 思考Request_List和ThreadID_List选择这样的数据结构的原因
    std::vector<pthread_t> ThreadID_List;      // pthread_t是线程的ID类型
    int Max_Requests;
    int Thread_Num;
    int Max_Thread_Num;
    bool Server_IsOn = true;

public:
    ThreadPool(int Thread_Num);
    ~ThreadPool();

    bool append(T *request);
    bool run();
    bool worker();
    void shutDown();
}

#endif