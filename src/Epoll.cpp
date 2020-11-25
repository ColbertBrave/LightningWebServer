#include "Epoll.h"
#include <errno.h>
#include <vector>
#include "WebServer.h" // TODO 只使用了Server_Run, extern能否满足要求
#include "HttpRequest.h"

const int EPOLL_WAIT_TIMEOUT = 10000;
const int MAX_EVENTS = 65536;

Epoll::Epoll(/* args */)
{
    this->EpollFd = epoll_create(5); //TODO 用epoll_create1还是epoll_create好
}

Epoll::~Epoll() {}

void Epoll::EpollAddEvent(int fd, struct epoll_event *event)
{
    if (epoll_ctl(EpollFd, EPOLL_CTL_ADD, fd, event) < 0)
    {
        LOG << "Failed to add event to epoll in the event loop:" << strerror(errno) << "\n";
    }
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

std::vector<std::shared_ptr<HttpRequest>> Epoll::GetReadyEvents()
{
    while (WebServer::Server_Run)
    {
        // Reactor模式. 一直阻塞直到有事件来临或满足超时条件
        int readyEventNums = epoll_wait(EpollFd, &(*ReadyEvents.begin()), MAX_EVENTS, EPOLL_WAIT_TIMEOUT)
        if (readyEventNums == -1)
        {
            LOG << "Epoll wait error" << strerror(error) << "\n";
        }

        // 将就绪事件封装成HttpRequest
        std::vector<std::shared_ptr<HttpRequest>> ReadyRequestsList;
        for (size_t i = 0; i < readyEventNums; i++)
        {
            int fd = ReadyEvents[i].data.fd;
            std::shared_ptr<HttpRequest> request;
            request->SetFd(fd);
            request->SetEvent(ReadyEvents[i]->events);
            ReadyRequestsList.embrace_back(request);
        }
        return ReadyRequestsList;
    }
}