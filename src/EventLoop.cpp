#include "EventLoop.h"

EventLoop::EventLoop(): Epoll(std::make_shared<Epoll>()), ThreadID(gettid())
{
    
}

EventLoop::~EventLoop() {}

// 添加事件给EventLoop进行监听
void EventLoop::AddRequest(std::shared_ptr<HttpRequest> request)
{
    // 要监听的事件类型由EventLoop指定给Epoll，Epoll只是一个工具人
    Epoll->EpollAddEvent(request->Fd, request->EventPtr);
}

// 修改请求
void EventLoop::ModifyRequest(std::shared_ptr<HttpRequest> request, epoll_event *event)
{
    Epoll->EpollModifyEvent(request->Fd, event);
}

// 删除请求
void EventLoop::DeleteRequest(std::shared_ptr<HttpRequest> request)
{
    Epoll->EpollDeleteEvent(request->Fd, request->EventPtr);
}

void EventLoop::StartLoop()
{
    assert(!WebServer::Server_Run);
    while (WebServer::Server_Run)
    {
        this->ReadyRequestsList = Epoll->GetReadyEvents();
        for (auto request : ReadyRequestsList)
        {
            // 一个线程对应一个EventLoop，因此不存在竞态条件
            // 虽然是request调用HandleRequest()去处理请求，但是HttpRequest同样也是工具人
            // HttpRequest的各种情形下处理函数由EventLoop传入
            request->HandleRequest();
        }
    }
}

// bool ThreadPool::Append(T *request)
// {
//     // 与从请求列表中取出请求出的互斥锁是同一把
//     pthread_mutex_lock(&mutex);
//     if (Request_List.size() >= ThreadPool::Max_Requests)
//     {
//         std::cout << "The number of requests exceeds the limit: " 
//                   << this->Max_Requests << std::endl;
//         return false;
//     }
//
//     Request_List.push_back(request);
//     pthread_mutex_unlock(&mutex);
//     return true;
// }

// void ThreadPool::Run()
// {
//     if (!Server_Status)
//     {
//         std::cout << "The server is not on" << std::endl;
//         throw std::exception();
//     }
//
//     /*
//         创建的所有线程均访问当前对象this的成员函数worker()
//         然后通过run()处理线程池对象的请求列表
//         不同线程同时处理该对象的请求列表，因此需要使用互斥锁
//     */
//     pthread_mutex_init(&Mutex, NULL);
//     while (Server_Status)
//     {
//         // 上锁的位置: 检测请求列表长度之前。
//         pthread_mutex_lock(&Mutex);             // 使用互斥锁会导致阻塞，思考如何优化
//         if (!Request_List.size())
//         {
//             continue;
//         }
//
//         T *firstRequest = Request_List.front();
//         Request_List.pop_front();
//         pthread_mutex_unlock(&Mutex);
//         firstRequest->process();
//     }
// }