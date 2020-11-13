#include "Epoll.h"

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

void Epoll::GetReadyEvents()
{
    int readyEventNums = epoll_wait(EpollFd, this->Ready_Events, MAX_EVENTS, Timeout) // Reactor模式. 一直阻塞直到有事件来临或满足超时条件
}