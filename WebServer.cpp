#include "WebServer.h"
#include "Utils.h"
#include "ThreadPool.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <functional>
#include <iostream>

// 构造函数只创建ThreadPool和NewRequest，不启动MainLoop和线程池
WebServer::WebServer(std::make_shared<EventLoop> loop): MainLoop(loop), ThreadPool(new ThreadPool)
                                                        NewRequest(std::make_shared<HttpRequest>()),
                                                        ListenFd(SetListenFd())
{
    // 由于此socket只监听有无连接，谈不上写和其他操作。故只有这两类。（默认是LT模式，即EPOLLLT |EPOLLIN）。
    // NewRequest->SetEvents(EPOLLIN | EPOLLET);       
    //TODO ??放在哪里更合适 DONE 这里就可以了，构造函数中没必要 DONE 改写结构，放在这里也没必要了 DONE 放哪里都可以
    NewRequest->SetEvent(EPOLLIN | EPOLLET);
    MainLoop->AddRequest(NewRequest);
    // 这里覆盖了NewRequest在构造时所拥有的ReadHandler和ConnHandler
    // TODO 这里也可以考虑通过继承关系virtual覆盖构造时默认的处理函数
    // 新的连接只可能是这两种事件类型，在EventLoop中处理新连接的handler为ReadHandler，
    // 因此在这里将DistributeNewRequest()绑定到NewRequest的ReadHandler
    // 将NewRequest交由MainLoop监听管理
    // 启动线程池以后由MainLoop接收并分发新的http请求
    NewRequest->SetReadHandler(std::bind(&WebServer::DistributeNewRequest, this));
    // 分发完新的请求以后，MainLoop继续监听并接收新的请求，因此设置ConnHandler为继续监听
    NewRequest->SetConnHandler(std::bind(&WebServer::DistributeNewRequest, this)); // TODO 待修改
}

WebServer::~WebServer()
{
    // TODO 等待到日志线程遍历保存所有缓冲区的日志
}

// 启动服务器: 启动线程池，MainLoop开始循环接收新的请求并分发请求
void WebServer::Start()
{
    ThreadPool->RunThreadPool();
    // 这个flag要设定在线程池启动后(?再斟酌一下)，StartLoop()之前，因此StartLoop()是不返回的 WRONG
    // 纠正 StartLoop()的终止和启动都是根据Server_Run这个全局标志的，即使放在StartLoop()之后，
    // 由于Server_Run尚未被设置为true，StartLoop()依然是会返回的，直到Server_Run改变了StartLoop()才运行起来
    WebServer::Server_Run = true;    
    MainLoop->StartLoop();
}

// TODO 这里的accept和epoll是否冲突？DONE 不冲突，accept是接收并建立连接，epoll是IO复用，这里是建立连接可能发生的套接字IO操作
// 纠正 epoll监听了listenfd，事件类型为EPOLLIN，相应的处理函数为DistributeNewRequest()，里面接收，封装并分发了请求
// 处理完毕后epoll继续监听listenfd
void WebServer::DistributeNewRequest()
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
        HttpRequest Request(newConnFd, clientAddr);
        Request->SetEvents(EPOLLIN | EPOLLET);       // TODO ??放在哪里更合适 DONE 这里就可以了，构造函数中没必要
        nextEventLoop->AddRequest(Request);       // TODO mainloop监听这个事件，还是放到各个loop去监听这件事情
    }
}


// WebServer::WebServer(int port = 8888) // TODO 在WebServer里将日志初始化 DONE 在main()里将日志初始化
// {
//     // TODO 在这之前首先启动日库线程 DONE 
//     // 对传入参数port进行检查
//     if (port > 65535 && port < 1024)
//     {
//         LOG << "Invalid server port\n";    // TODO 改为写入到日志中 DOING 未测试
//         throw std::exception()
//     }
//
//     bzero(Server_Addr, sizeof(*Server_Addr));       // TODO 用C++特性改写 DOING bzero比memeset更好(虽然只支持Linux)
//     this->Server_Addr->sin_family = AF_INET;        // IPv4协议族
//     this->Server_Addr->sin_port = htons(port);
//     this->Server_Addr->sin_addr->s_addr = htonl(INADDR_ANY);
//     this->Listen_Fd = socket(PF_INET, SOCK_STREAM, 0);
//
//     // 允许重用本地地址和端口(结束连接后可以立即重启)
//     int flag = 1;
//     setsockopt(Listen_Fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
//
//     if (bind(this->Listen_Fd, Server_Addr, typeof(Server_Addr)) < 0)
//     {
//         LOG <<"Failed to bind server address.\n";
//         throw std::exception();
//     }
// }