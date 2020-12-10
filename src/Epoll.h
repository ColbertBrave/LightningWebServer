#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>
#include <errno.h>

#include <vector>
#include <memory>

#include "Timer/Timer.h"

class EventLoop;
class Epoll
{
private:
    int EpollFd;
    std::shared_ptr<EventLoop>      EventloopPtr;
    std::shared_ptr<TimerQueue>     TimerQueuePtr;
    std::vector<epoll_event*>       ReadyEvents;

public:
    Epoll();
    ~Epoll();

    void EpollAddEvent(std::shared_ptr<HttpRequest> request, int timeout);
    void EpollModifyEvent(int fd, struct epoll_event* event);
    void EpollDeleteEvent(int fd, struct epoll_event* event);
    void SetEventloopPtr(std::shared_ptr<EventLoop> eventloopPtr);
    void RemoveExpiredEvent();
    std::vector<std::shared_ptr<HttpRequest>> GetReadyEvents();
};  

#endif