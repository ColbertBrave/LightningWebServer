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

    bool EpollAddEvent();
    bool EpollModifyEvent();
    bool EpollDeleteEvent();
    void GetReadyEvents();
};  

#endif