#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>
#include <vector>
#include <memory>
#include <errno.h>
#include "Timer/Timer.h"

class Epoll
{
private:
    int EpollFd;
    std::shared_ptr<TimerQueue> TimerQueuePtr;
    std::vector<epoll_event*> ReadyEvents;

public:
    Epoll();
    ~Epoll();

    void EpollAddEvent(std::shared_ptr<HttpRequest> request, int timeout);
    void EpollModifyEvent(int fd, struct epoll_event* event);
    void EpollDeleteEvent(int fd, struct epoll_event* event);
    void RemoveExpiredEvent();
    std::vector<std::shared_ptr<HttpRequest>> GetReadyEvents();
};  

#endif