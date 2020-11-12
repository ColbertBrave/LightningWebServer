#include "WebServer.h"
#include "Utils.h"
#include "ThreadPool.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <iostream>


WebServer::WebServer(int port = 8888) // TODO 在WebServer里将日志初始化
{
    // TODO 在这之前首先启动日库线程
    // 对传入参数port进行检查
    if (port > 65535 && port < 1024)
    {
        LOG << "Invalid server port\n";    // TODO 改为写入到日志中 DOING 未测试
        throw std::exception()
    }

    bzero(Server_Addr, sizeof(*Server_Addr));       // TODO 用C++特性改写
    this->Server_Addr->sin_family = AF_INET;        // IPv4协议族
    this->Server_Addr->sin_port = htons(port);
    this->Server_Addr->sin_addr->s_addr = htonl(INADDR_ANY);
    this->Listen_Fd = socket(PF_INET, SOCK_STREAM, 0);

    // 允许重用本地地址和端口(结束连接后可以立即重启)
    int flag = 1;
    setsockopt(Listen_Fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    if (bind(this->Listen_Fd, Server_Addr, typeof(Server_Addr)) < 0)
    {
        LOG <<"Failed to bind server address.\n";
        throw std::exception();
    }
}

// 构造函数只创建线程池，但不启动MainLoop和线程池
WebServer::WebServer(std::make_shared<EventLoop> loop): MainLoop(loop), ThreadPool(new ThreadPool) {}

WebServer::~WebServer()
{
    // TODO 等待到日志线程遍历保存所有缓冲区的日志
}

// 启动服务器: 启动线程池，接收新的连接并交由MainLoop监听
void WebServer::Start()
{
    ThreadPool->RunThreadPool();
    // MainLoop接收新连接请求并分发请求
    // 由于此socket只监听有无连接，谈不上写和其他操作。故只有这两类。（默认是LT模式，即EPOLLLT |EPOLLIN）。
    //NewRequest->SetEvents(EPOLLIN | EPOLLET);       //TODO ??放在哪里更合适 DONE 这里就可以了，构造函数中没必要 DONE 改写结构，放在这里也没必要了
    ListenNewRequest();
    WebServer::Server_Run = true;
}

// TODO 这里的accept和epoll是否冲突？DONE 不冲突，accept是接收并建立连接，epoll是IO复用，这里是建立连接可能发生的套接字IO操作
void WebServer::ListenNewRequest()
{
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    socklen_t clientAddrLength = sizeof(clientAddr);
    int newConnFd;
    while ((newConnFd = accept(this->ListenFd, (struct sockaddr*)&clientAddr, &clientAddrLength)) > 0)
    {
        // 当超过服务器最大连接数时阻塞等待/关闭
        if (newConnFd > MAX_CONNECTIONS)
        {
            //...
        }

        // 设置为非阻塞模式 WHY DONE Epoll是多路复用IO，如果设置成阻塞，那么任意一个连接都可能阻塞整个epoll，因此IO多路复用必须配合非阻塞IO
        // More: https://www.zhihu.com/question/49947156
        if (!SetSocketNonBlocking(newConnFd))
        {
            LOG << "Failed to set the new conn fd non-blocking\n";
            close(newConnFd);
        }

        // 封装新连接为请求, 取出一个EventLoop, 然后分发请求给它
        std::shared_ptr<EventLoop> nextEventLoop = ThreadPool->GetNextEventLoop();
        HttpRequest newRequest(newConnFd, clientAddr);
        NewRequest->SetEvents(EPOLLIN | EPOLLET);       // TODO ??放在哪里更合适 DONE 这里就可以了，构造函数中没必要
        nextEventLoop->AddRequest(newRequest);       // TODO mainloop监听这个事件，还是放到各个loop去监听这件事情
    }
}