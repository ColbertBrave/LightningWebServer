#include "Epoll.h"
#include <errno.h>
#include <vector>
#include "WebServer.h" // TODO 只使用了Server_Run, extern能否满足要求
#include "HttpRequest.h"
#include <cstring>

const int EPOLL_WAIT_TIMEOUT = 10000;
const int MAX_EVENTS = 65536;

Epoll::Epoll(): TimerQueuePtr(std::make_shared<TimerQueue>())
{
    this->EpollFd = epoll_create(5); //TODO 用epoll_create1还是epoll_create好
}

Epoll::~Epoll() {}

// 添加事件给epoll监听，同时设置一个定时器，在设定时间内无响应则视作过期时间予以移除
void Epoll::EpollAddEvent(std::shared_ptr<HttpRequest> request, int timeout)
{
    TimerNode node(request, timeout);
    if (epoll_ctl(EpollFd, EPOLL_CTL_ADD, request->Fd, request->EventPtr) < 0)
    {
        LOG << "Failed to add event to epoll in the event loop:" << strerror(errno) << "\n";
    }

    // 添加成功了再加入小根堆
    TimerQueuePtr->AddTimerNode(node);
}

void Epoll::EpollModifyEvent(int fd, struct epoll_event *event)
{
    if (epoll_ctl(EpollFd, EPOLL_CTL_MOD, fd, event) < 0)
    {
        LOG << "Failed to modify event in the event loop:" << strerror(errno) << "\n";
    }
}

void Epoll::EpollDeleteEvent(int fd, struct epoll_event *event)
{
    if (epoll_ctl(EpollFd, EPOLL_CTL_DEL, fd, event) < 0)
    {
        LOG << "Failed to delete event in the event loop:" << strerror(errno) << "\n";
    }
}

// TODO是否有必要在关闭定时器时定时器队列里request->CloseHttp()
void Epoll::RemoveExpiredEvent()
{
    TimerQueuePtr->RemoveExpiredTimerNode();
}

void SetEventloopPtr(std::shared_ptr<EventLoop> eventloopPtr)
{
    this->EventLoopPtr = eventloopPtr;
}

std::vector<std::shared_ptr<HttpRequest>> Epoll::GetReadyEvents()
{
    while (WebServer::Server_Run)
    {
        // Reactor模式. 一直阻塞直到有事件来临或满足超时条件
        int readyEventNums = epoll_wait(EpollFd, &(*ReadyEvents.begin()), MAX_EVENTS, EPOLL_WAIT_TIMEOUT);
        if (readyEventNums == -1)
        {
            LOG << "Epoll wait error" << strerror(errno) << "\n";
        }

        // 将就绪事件封装成HttpRequest
        std::vector<std::shared_ptr<HttpRequest>> ReadyRequestsList;
        for (size_t i = 0; i < readyEventNums; i++)
        {
            int fd = ReadyEvents[i]->data.fd;
            std::shared_ptr<HttpRequest> request = std::make_shared<HttpRequest>();
            request->SetEventloopPtr(EventloopPtr); 
            // TODO 这里有问题，在request构造时就需要一个明确的EventLoopPtr了
            request->SetFd(fd);
            request->SetEvent(ReadyEvents[i]->events);
            ReadyRequestsList.push_back(request);
        }
        return ReadyRequestsList;
    }
}