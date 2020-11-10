#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <memory>
#include "Worker.h"
#include <vector>


const int MAX_THREAD_NUM = 16;
class ThreadPool
{
private:
    //std::list<HttpRequest*>          RequestList;        // 思考Request_List和ThreadID_List选择这样的数据结构的原因
    //static const int            MAXThreadNum = 16;       // 作为特例，有序型的const静态数据成员可以在类体中用一常量值初始化
    std::vector<std::shared_ptr<Worker>>         WorkerList;    // TODO 其他结构如何: array
    std::vector<std::shared_ptr<EventLoop>>      EventLoopList;
    unsigned int                                 NextLoopIndex;
    unsigned int                                 ThreadNum;
    std::shared_ptr<EventLoop>                   MainLoop;
    
    
    static unsigned int         MaxRequests;
    
public:
    ThreadPool(int Thread_Num);
    ~ThreadPool();

    void RunThreadPool();
    std::shared_ptr<EventLoop> GetNextEventLoop();
} 

#endif