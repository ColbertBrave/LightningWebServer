#include "EventLoop.h"

EventLoop::EventLoop(): Epoll(std::make_shared<Epoll>()), ThreadID(gettid())
{
    
}

EventLoop::~EventLoop() {}

// 添加事件给EventLoop进行监听
void EventLoop::AddRequest(std::shared_ptr<HttpRequest> request)
{
    // 要监听的事件类型由EventLoop指定给Epoll，Epoll只是一个工具人
    Epoll->EpollAddEvent(request->Fd, );
}

// 修改请求
void EventLoop::ModifyRequest(std::shared_ptr<HttpRequest> request, epoll_event *event)
{
    Epoll->EpollModifyEvent(request->Fd, );
}

// 删除请求
void EventLoop::DeleteRequest(std::shared_ptr<HttpRequest> request)
{
    Epoll->EpollDeleteEvent(request->Fd);
}

void EventLoop::StartLoop()
{
    assert(!WebServer::Server_Run);
    while (WebServer::Server_Run)
    {
        this->Events_List = Epoll->GetReadyEvents();
        for (auto request : Events_List)
        {
            // 一个线程对应一个EventLoop，因此不存在竞态条件
            // 虽然是request调用HandleRequest()去处理请求，但是HttpRequest同样也是工具人
            // HttpRequest的各种情形下处理函数由EventLoop传入
            request->HandleRequest();
        }
    }
}

/*
    HttpRequest中并不具有各种情形下的处理方法，这些处理方法需要从外界传入
    服务器ListenFd监听接收到的新连接的处理方法在WebServer中传入
    新的连接NewConnFd上监听到请求处理方法在EventLoop中传入
*/
bool EventLoop::DealReadEvent(int sockFd)      // 对可读文件进行处理：加入到请求队列中分配线程进行处理
{
    // 读操作，从连接队列中取出该连接添加至线程池的请求列表中，由线程池分配线程进行处理
    HttpRequest *theHttpConn = httpConnQueue[sockFd];
    // 还需要设置该HttpRequest对象所需的操作/状态，以便线程进行处理
    (*threadPool).append(theHttpConn);
    return true;
}


bool EventLoop::DealWriteEvent(int sockFd)
{
    // 写操作，从连接队列中取出该连接添加至线程池的请求列表中，由线程池分配线程进行处理
    HttpRequest *theHttpConn = httpConnQueue[sockFd];
    // 还需要设置该HttpRequest对象所需的操作/状态，以便线程进行处理
    (*threadPool).append(theHttpConn);
    return true;
}

// 处理异常事件
bool EventLoop::DealAbnormalEvent(int sockFd)
{
    HttpRequest *theHttpConn = httpConnQueue[sockFd];
    theHttpConn->close();
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