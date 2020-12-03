#include "ThreadPool.h"
#include <pthread.h>
#include <iostream>
const int MAX_THREADS = 16;

ThreadPool::ThreadPool(std::shared_ptr<EventLoop> loop, int threadNum): MainLoop(loop), ThreadNum(threadNum), NextLoopIndex(0)
{
    if (ThreadNum<=0 || ThreadNum> MAX_THREADS)
    {
        LOG << "The num of threads is out of range.\n";
    }
}

ThreadPool::~ThreadPool() {}

void ThreadPool::RunThreadPool()
{
    for (size_t i = 0; i < ThreadNum; i++)
    {
        // 每个Worker线程一旦被构造就开始工作
        std::shared_ptr<Worker> workman = std::make_shared<Worker>();
        WorkerList.emplace_back(workman);
    } //OLD 换用array以后没有必要了
    
    // 每个Worker线程一旦被构造就开始工作
    //WorkerList.fill(std::make_shared<Worker>());
    for (auto i = 0; i < ThreadNum; i++)
    {
        // 使用at不使用[]的原因: 带有越界检查
        EventLoopList.at(i) = WorkerList.at(i)->ReturnEventLoopPtr();
    }
}

std::shared_ptr<EventLoop> ThreadPool::GetNextEventLoop()
{
    if (!EventLoopList.empty())
    {
        std::shared_ptr<EventLoop> nextLoop = EventLoopList[NextLoopIndex];
        NextLoopIndex = (NextLoopIndex + 1) % ThreadNum;
        return nextLoop;
    }
}

// ThreadPool::ThreadPool(int thread_num = 12, int max_requests = 10000): ThreadNum(thread_num), MaxRequests(max_requests)
// {
//     if (thread_num <= 0 || thread_num > Max_Thread_Num)
//     {
//         std::cout << "The number of created threads: "<< thread_num
//                   << "is less than 0 or too large" << std::endl;
//         thread_num = 12;
//     }
//     // TODO 线程池里创建一定数量的线程，每个线程拥有一个eventloop和一个eventslist，线程轮询处理请求
//     // TODO 新产生的一个请求，如何选择分配的线程eventslist？
//     // TODO 这里应当是创建一定数量的Worker对象
//     ThreadID_List = vector<pthread_t>(thread_num, 0);   // 亦可使用auto count:counts的C++11特性
//     for (int i = 0; i < thread_num; i++)
//     {
//         if (pthread_create(&ThreadID_List[i], NULL, work, this) != 0)  // 创建的每一个线程运行函数均为worker
//         {
//             std::cout << "Error occured when initing the thread pool" << std::endl;
//             // 创建失败后是否有必要清除已赋值的ThreadID_List或抛出异常后继续执行
//             throw std::exception();
//         }
//         if (pthread_detach(&ThreadID_List[i]) != 0)
//         {
//             std::cout << "Error occured when detaching threads" << std::endl;
//             throw std::exception();
//         }
//     }
// }