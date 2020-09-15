#include "ThreadPool.h"
#include <pthread.h>
#include <iostream>

/*
*    构造函数: 根据设定的线程数量创建线程池
*    输入参数: 线程数量；如果未输入则默认为12
*    返 回 值: void
*/
template <typename T>
ThreadPool::ThreadPool(int thread_num = 12): Thread_Num(thread_num)
{
    if (thread_num <= 0 || thread_num > Max_Thread_Num)
    {
        std::cout << "The number of created threads: "<< thread_num
                  << "is less than 0 or too large" << std::endl;
        thread_num = 12;
    }

    (*this).ThreadID_List = vector<pthread_t>(thread_num, 0);
    for (int i = 0; i < thread_num; i++)
    {
        if (pthread_create(&ThreadID_List[i], NULL, worker, this) != 0)  // 创建的每一个线程运行函数均为worker，参数为this，这里有很多问题
        {
            std::cout << "Error occured when initing the thread pool"
                      << std::endl;                                      // 创建失败后是否有必要清除已赋值的ThreadID_List或抛出异常后继续执行
            throw std::exception();
        }

        if (pthread_detach(&ThreadID_List[i]) != 0)
        {
            std::cout << "Error occured when detaching threads" << std::endl;
            throw std::exception();
        }
    }      
}

template <typename T>
ThreadPool::~ThreadPool() 
{}

template <typename T>
bool ThreadPool::append(T *request)
{
    if (Request_List.size() >= Max_Requests)
    {
        std::cout << "The number of requests exceeds the limit: " 
                  << this->Max_Requests << std::endl;
        return false;
    }

    Request_List.push_back(request);
    return true;
}

template <typename T>
bool ThreadPool::run()
{
    if (!Server_IsOn)
    {
        std::cout << "The server is not on" << std::endl;
        throw std::exception();
    }

    while (Server_IsOn)
    {
        if (!Request_List.size())
        {
            continue;
        }

        T *firstRequest = Request_List.front();
        Request_List.pop_front();
        firstRequest->process();
    }
}

template <typename T>
void ThreadPool::shutDown()
{
    this->Server_IsOn = false;
    std::cout << "Server has been shut down" << std::endl;
}

template <typename T>
bool worker(void *args)
{
    ThreadPool *threadPool = static_cast<ThreadPool *>(args);
    threadPool->run();
    return true;
}


