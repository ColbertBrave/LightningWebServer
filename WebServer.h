#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>
#include <memory>
#include "ThreadPool.h"
#include "HttpRequest.h"
#include "Logging/Logging.h"
#include "EventLoop.h"

#define MAX_EVENTS_NUMBER 20000
#define MAX_CONNECTIONS 65536
class WebServer
{
private:
    int                             ListenFd;
    std::unique_ptr<ThreadPool>     ThreadPool;
    std::shared_ptr<EventLoop>      MainLoop;
    //std::shared_ptr<HttpRequest>    NewRequest;


    void ListenNewRequest();

    // vector<HttpRequest *> httpConnQueue(MAX_CONNECTIONS, 0);
    // epoll_event Ready_Events[MAX_EVENTS_NUMBER];   
    // // 存储就绪的事件, 一次就绪数量有限制，为什么要这么设置？是否可以设置成动态的，单个事件是否可以使用auto类型
public:
    WebServer();
    ~WebServer();

    void Start();

public:
    struct sockaddr_in *Server_Addr;
    static bool         Server_Run;
}

#endif