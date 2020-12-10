#include <cassert>
#include <memory>

#include "EventLoop.h"
#include "WebServer.h"


EventLoop::EventLoop(): EpollPtr(std::make_shared<Epoll>()), ThreadID(gettid())
{
    // 构造时即将Epoll和EventLoop关联起来，避免后续可能出现的空指针情况
    EpollPtr->SetEventloopPtr(shared_from_this());
}


// 添加事件给EventLoop进行监听
void EventLoop::AddRequest(std::shared_ptr<HttpRequest> request, int timeout)
{
    // 要监听的事件类型由EventLoop指定给Epoll，Epoll只是一个工具人
    EpollPtr->EpollAddEvent(request, timeout);
}

// 修改请求
void EventLoop::ModifyRequest(std::shared_ptr<HttpRequest> request, epoll_event *event)
{
    EpollPtr->EpollModifyEvent(request->Fd, event);
}

// 删除请求
void EventLoop::DeleteRequest(std::shared_ptr<HttpRequest> request)
{
    EpollPtr->EpollDeleteEvent(request->Fd, request->EventPtr);
}

void EventLoop::StartLoop()
{
    //assert(!WebServer::Server_Run);
    while (WebServer::Server_Run) // 全局flag
    {
        // 考虑下std::move, std::copy, std::swap等
        this->ReadyRequestsList = EpollPtr->GetReadyEvents();
        for (auto request : ReadyRequestsList)
        {
            // 一个线程对应一个EventLoop，因此不存在竞态条件
            // 虽然是request调用HandleRequest()去处理请求，但是HttpRequest同样也是工具人
            // HttpRequest的各种情形下处理函数由EventLoop传入
            request->HandleRequest();
        }
        this->EpollPtr->RemoveExpiredEvent();
        // TimerNode析构时会调用httprequest的closehttp()方法
        // TimerNode何时析构？
    }
}