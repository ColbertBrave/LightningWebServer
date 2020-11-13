#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <pthread.h>
#include <memory>
#include "Worker.h"
#include <vector>
#include <array>
const int MAX_THREAD_NUM = 16;
class ThreadPool
{
private:
    //std::list<HttpRequest*>          RequestList;        // 思考Request_List和ThreadID_List选择这样的数据结构的原因
    //static const int            MAXThreadNum = 16;       // 作为特例，有序型的const静态数据成员可以在类体中用一常量值初始化
    // 线程Worker的数量是固定的，因此没有使用vector
    // 在使用vector时需要依次make_shared and push_back/embrace_back
    // when using std::array, we can use fill() instead of the complex operation
    // OLD std::vector<std::shared_ptr<Worker>>         WorkerList;    // TODO 其他结构如何: array DONE array更好
    std::array<std::shared_ptr<Worker>, ThreadNum>      WorkerList;
    std::array<std::shared_ptr<EventLoop>, ThreadNum>   EventLoopList;
    unsigned int                                        NextLoopIndex;
    unsigned int                                        ThreadNum;
    std::shared_ptr<EventLoop>                          MainLoop;
    
    
    static unsigned int         MaxRequests;
    
public:
    ThreadPool(std::shared_ptr<EventLoop> loop, int threadNum = 12);
    ~ThreadPool();

    void                        RunThreadPool();
    std::shared_ptr<EventLoop>  GetNextEventLoop();
}
#endif