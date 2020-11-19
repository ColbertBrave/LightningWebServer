#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#define gettid() syscall(SYS_gettid)

#include <sys/types.h>
#include <pthread.h>
#include "HttpRequest.h"
#include "Epoll.h"
#include <vector>

/*
    EventLoop类负责监听和返回就绪事件列表，并且提供对应的事件处理函数接口，
    由封装好的线程类负责调用和运行函数
    EventLoop和Worker可以视作劳心者与劳力者。EventLoop负责接收请求，并让线程来处理相应的请求
*/
class EventLoop
{
private:
    std::shared_ptr<Epoll>      Epoll;
    pid_t                       ThreadID;
    std::vector<HttpRequest>    ReadyEvents;
    const size_t                MAX_EVENTS;
    int                         Timeout;
    

public:
    // TODO 是否需要增加拷贝构造函数和拷贝赋值运算符
    EventLoop();
    ~EventLoop();

    void AddRequest(std::shared_ptr<HttpRequest> request);
    void ModifyRequest();
    void DeleteRequest();
    void StartLoop();


};
#endif