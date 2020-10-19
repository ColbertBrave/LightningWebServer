#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>
#include "ThreadPool.h"
#include "HttpRequest.h"

#define MAX_EVENTS_NUMBER 20000
#define MAX_CONNECTIONS 65536
class WebServer
{
public:
    WebServer();
    ~WebServer();

    bool Init();
    void EventLoop();
    bool BuildNewConnect();
    bool DealReadEvent(int sockFd);
    bool DealWriteEvent(int sockFd);
    bool DealAbnormalEvent(int sockFd);

public:
    struct sockaddr_in *Server_Addr;


private:
    int Listen_Fd;          // 监听套接字  
    int Epoll_Fd;
    ThreadPool *threadPool; // 该服务器对应的线程池
    bool Server_IsOn;
    vector<HttpRequest *> httpConnQueue(MAX_CONNECTIONS, 0);

    epoll_event Ready_Events[MAX_EVENTS_NUMBER];   
    // 存储就绪的事件, 一次就绪数量有限制，为什么要这么设置？是否可以设置成动态的，单个事件是否可以使用auto类型
}

#endif