#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>


class Epoll
{
private:
    int EpollFd;
    

public:
    Epoll(/* args */);
    ~Epoll();

    void EpollAddEvent();
    void EpollModifyEvent();
    void EpollDeleteEvent();
    void GetReadyEvents();
};  

#endif