#include "ThreadPool.h"
#include <pthread.h>
#include <iostream>

/*
*    构造函数: 根据设定的线程数量创建线程池
*    输入参数: 线程数量；如果未输入则默认为12
*    返 回 值: void
*/
template <typename T>
ThreadPool::ThreadPool(int thread_num = 12, int max_requests = 10000): Thread_Num(thread_num), Max_Requests(max_requests)
{
    if (thread_num <= 0 || thread_num > Max_Thread_Num)
    {
        std::cout << "The number of created threads: "<< thread_num
                  << "is less than 0 or too large" << std::endl;
        thread_num = 12;
    }

    (*this).ThreadID_List = vector<pthread_t>(thread_num, 0);   // 亦可使用auto count:counts的C++11特性
    for (int i = 0; i < thread_num; i++)
    {
        if (pthread_create(&ThreadID_List[i], NULL, worker, this) != 0)  // 创建的每一个线程运行函数均为worker
        {
            std::cout << "Error occured when initing the thread pool" << std::endl;
                                                            // 创建失败后是否有必要清除已赋值的ThreadID_List或抛出异常后继续执行
            throw std::exception();
        }

        if (pthread_detach(&ThreadID_List[i]) != 0)
        {
            std::cout << "Error occured when detaching threads" << std::endl;
            throw std::exception();
        }
    }
    this->Server_IsOn = true;
}


template <typename T>
ThreadPool::~ThreadPool() 
{}


/*
    work()应当为一个静态函数，目的是不管是否创建了对象，都可以调用worker函数？
    静态函数只能调用静态数据成员和静态函数
    为了调用类的动态成员，因此pthread_create里的函数参数为类的当前对象(this)，见P304
    创建的每个线程在创建后均运行worker(), 该函数运行线程池，
    从请求列表中取出头部请求，并处理请求
    静态函数会被自动分配在一个一直使用的存储区，直到退出应用程序实例
    避免了调用函数时压栈出栈，速度快很多。 
*/
template <typename T>
bool worker(void *args)
{
    ThreadPool *threadPool = static_cast<ThreadPool *>(args);
    if (!threadPool)
    {
        throw std::exception();
        return false;
    }
    threadPool->run();
    return true;
}


template <typename T>
void ThreadPool::run()
{
    if (!Server_IsOn)
    {
        std::cout << "The server is not on" << std::endl;
        throw std::exception();
    }

    /*
        创建的所有线程均访问当前对象this的成员函数worker()
        然后通过run()处理线程池对象的请求列表
        不同线程同时处理该对象的请求列表，因此需要使用互斥锁
    */
    pthread_mutex_init(&mutex, NULL);
    while (Server_IsOn)
    {
        // 上锁的位置: 检测请求列表长度之前。
        pthread_mutex_lock(&mutex);             // 使用互斥锁会导致阻塞，思考如何优化
        if (!Request_List.size())
        {
            continue;
        }

        T *firstRequest = Request_List.front();
        Request_List.pop_front();
        pthread_mutex_unlock(&mutex);
        firstRequest->process();
    }
}

template <typename T>
bool ThreadPool::append(T *request)
{
    // 与从请求列表中取出请求出的互斥锁是同一把
    pthread_mutex_lock(&mutex);
    if (Request_List.size() >= Max_Requests)
    {
        std::cout << "The number of requests exceeds the limit: " 
                  << this->Max_Requests << std::endl;
        return false;
    }

    Request_List.push_back(request);
    pthread_mutex_unlock(&mutex);
    return true;
}

template <typename T>
void ThreadPool::shutDown()
{
    this->Server_IsOn = false;
    pthread_mutex_destroy(&mutex);
    std::cout << "Server has been shut down" << std::endl;
}




