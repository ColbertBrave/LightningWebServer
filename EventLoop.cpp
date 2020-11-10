#include "EventLoop.h"

EventLoop::EventLoop(/* args */)
{
    this->EpollFd = epoll_create(5); //TODO 用epoll_create1还是epoll_create好
    this->ThreadID = gettid();
    
}

EventLoop::~EventLoop() {}

// 添加事件给EventLoop进行监听
void EventLoop::AddRequest(std::shared_ptr<HttpRequest> request)
{
    if (epoll_ctl(EpollFd, EPOLL_CTL_ADD, request->Fd, request->EventPtr) < 0)
    {
        LOG << "Failed to add event to epoll in the event loop:" << strerror(errno) << "\n";
    }
}

// 修改请求
void EventLoop::ModifyRequest(std::shared_ptr<HttpRequest> request, epoll_event *event)
{
    if (epoll_ctl(EpollFd, EPOLL_CTL_MOD, request->Fd, event) < 0)
    {
        LOG << "Failed to modify event in the event loop:" << strerror(errno) << "\n";
    }
}

void EventLoop::DeleteRequest(std::shared_ptr<HttpRequest> request)
{
    if (epoll_ctl(EpollFd, EPOLL_CTL_DEL, request->Fd, request->EventPtr) < 0)
    {
        LOG << "Failed to delete event in the event loop:" << strerror(errno) << "\n";
    }
}

void EventLoop::StartLoop()
{
    while (WebServer::Server_Run)
    {
        int readyEventNums = epoll_wait(this->Epoll_Fd, this->Ready_Events, MAX_EVENTS, Timeout) // Reactor模式. 一直阻塞直到有事件来临或满足超时条件
        for (size_t i = 0; i < readyEventNums; i++)
        {
            
            
        }
        
    }
}

void EventLoop::StartLoop()
{
    assert(!WebServer::Server_Run);
    while (WebServer::Server_Run)
    {
        this->Events_List = GetReadyEvents();
        for (auto request : Events_List)
        {
            // 一个线程对应一个EventLoop，因此不存在竞态条件
            request->HandleRequest();
        }
    }
}

void WebServer::StartLoop()
{
    while (Server_Run)
    {
        int readyEventNums = epoll_wait(this->Epoll_Fd, this->Ready_Events, MAX_EVENTS_NUMBER, timeout) // Reactor模式. 一直阻塞直到有事件来临或满足超时条件
        if (readyEventNums < 0 && errno != EINTR)   // ? errno != EINTR
        {
            // LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (auto event:Ready_Events)
        {
            // 依次取出就绪I/O事件并进行处理
            int sockFd = event.data.fd;

            // 此处可以优化
            // 有新连接
            if (sockFd == this->Listen_Fd)      
            {
                // 所有事件都先经历建立连接这一步
                BuildNewConnect();
            }
            else
            {
                if (sockFd & EPOLLIN)  // 此时与sockFd关联的文件可读
                {
                    DealReadEvent(sockFd);   // 对可读文件进行处理
                }

                if (sockFd & EPOLLOUT)  // 此时与sockFd关联的文件可写
                {
                    DealWriteEvent(sockFd);
                }

                if (sockFd & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))  // 此时与sockFd关联文件发生错误，挂断或半挂断
                {
                    DealAbnormalEvent(sockFd);
                }
            }            
        }
    } 
}

bool WebServer::DealReadEvent(int sockFd)      // 对可读文件进行处理：加入到请求队列中分配线程进行处理
{
    // 读操作，从连接队列中取出该连接添加至线程池的请求列表中，由线程池分配线程进行处理
    HttpRequest *theHttpConn = httpConnQueue[sockFd];
    // 还需要设置该HttpRequest对象所需的操作/状态，以便线程进行处理
    (*threadPool).append(theHttpConn);
    return true;
}



bool WebServer::DealWriteEvent(int sockFd)
{
    // 写操作，从连接队列中取出该连接添加至线程池的请求列表中，由线程池分配线程进行处理
    HttpRequest *theHttpConn = httpConnQueue[sockFd];
    // 还需要设置该HttpRequest对象所需的操作/状态，以便线程进行处理
    (*threadPool).append(theHttpConn);
    return true;
}

// 处理异常事件
bool WebServer::DealAbnormalEvent(int sockFd)
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