#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#define gettid() syscall(SYS_gettid)

#include <sys/types.h>
#include <pthread.h>

#include <vector>

#include "HttpRequest.h"
#include "Epoll.h"

const int DEFAULT_LIVE_TIME = 2 * 60 * 1000; // 2分钟
/*
    EventLoop类负责监听和返回就绪事件列表，并且提供对应的事件处理函数接口，
    由封装好的线程类负责调用和运行函数
    EventLoop和Worker可以视作劳心者与劳力者。EventLoop负责接收请求，并让线程来处理相应的请求
*/
class EventLoop: public std::enable_shared_from_this<EventLoop>
{
private:
    std::shared_ptr<Epoll>      EpollPtr;
    pid_t                       ThreadID;
    std::vector<std::shared_ptr<HttpRequest>>    ReadyRequestsList;
    const size_t                MAX_EVENTS = 10000;
    int                         Timeout;
    

public:
    // TODO 是否需要增加拷贝构造函数和拷贝赋值运算符
    EventLoop();
    ~EventLoop() = default;

    void AddRequest(std::shared_ptr<HttpRequest> request, int timeout = DEFAULT_LIVE_TIME);
    void ModifyRequest(std::shared_ptr<HttpRequest> request, epoll_event *event);
    void DeleteRequest(std::shared_ptr<HttpRequest> request);
    void StartLoop();
};
#endif