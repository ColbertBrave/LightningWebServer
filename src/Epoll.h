#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>
#include <vector>
#include "Timer/Timer.h

class Epoll
{
private:
    int EpollFd;
    std::vector<epoll_event *> ReadyEvents;

public:
    Epoll(/* args */);
    ~Epoll();

    void EpollAddEvent();
    void EpollModifyEvent();
    void EpollDeleteEvent();
    std::vector<std::shared_ptr<HttpRequest>> GetReadyEvents();
};  

#endif