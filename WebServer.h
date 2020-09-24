#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "ThreadPool.h"

#define MAX_EVENTS_NUMBER 20000
class WebServer
{
public:
    WebServer();
    ~WebServer();

    bool Init();
    void EventLoop();
    bool DealReadEvent();
    bool BuildNewConnect();

public:
    struct sockaddr_in *Server_Addr;


private:
    int Listen_Fd;      
    int Epoll_Fd;
    ThreadPool *threadPool; // 该服务器对应的线程池
    bool Server_IsOn;

    epoll_event Ready_Events[MAX_EVENTS_NUMBER];   
    // 存储就绪的事件, 一次就绪数量有限制，为什么要这么设置？是否可以设置成动态的，单个事件是否可以使用auto类型
}

#endif