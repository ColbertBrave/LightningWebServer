#include "Worker.h"
#include <cassert>
#include <pthread.h>

Worker::Worker(): Eventloop(std::make_shared<EventLoop>())
{
    if (pthread_create(&ThreadID, NULL, Run, this))
    {
        // 写入日志但是不抛出异常，如果只是一条线程创建失败并不影响服务器运行
        LOG << "Failed to create a thread\n"; 
    }
    pthread_detach(ThreadID);
}

Worker::~Worker() {}

/*
    work()应当为一个静态函数，目的是不管是否创建了对象，都可以调用worker函数？
    静态函数只能调用静态数据成员和静态函数
    为了调用类的动态成员，因此pthread_create里的函数参数为类的当前对象(this)，见P304
    创建的每个线程在创建后均运行worker(), 该函数运行线程池，
    从请求列表中取出头部请求，并处理请求
    静态函一直使数会被自动分配在一个用的存储区，直到退出应用程序实例
    避免了调用函数时压栈出栈，速度快很多。 
*/
void* Worker::Run(void *args)      // NOTE 在h文件中声明的静态成员函数，在cpp文件无需加上static关键字
{
    Worker *workerPtr = static_cast<Worker *>(args);
    if (!workerPtr)
    {
        throw std::exception();
    }
    workerPtr->WorkerThreadFunc();
    return NULL;
}

// Worker被创建后即开始startLoop
void Worker::WorkerThreadFunc()
{
    while (Server_Status)   // Server_Status是一个全局标志
    {
        EventLoop->startLoop();
    }
}

std::shared_ptr<EventLoop> Worker::ReturnEventLoopPtr()
{
    return this->EventLoop;
}
