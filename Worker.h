#ifndef WORKER_H
#define WORKER_H
#include <vector>
#include <memory>
#include <pthread.h>
#include <sys/types.h>
#include "MutexLock.h"
#include "Logging/Logging.h"
#include "EventLoop.h"

// EventLoop创建了之后线程才能够跑起来
// 每个
class Worker
{
private:
    pid_t                       ThreadID;
    std::shared_ptr<EventLoop>  EventLoop;

    static void* Run(void* args);
    void WorkerThreadFunc();
    
public:
    // 构造时也会同时构造所拥有的eventloop
    Worker();
    ~Worker();

    std::shared_ptr<EventLoop> ReturnEventLoopPtr();
};
#endif