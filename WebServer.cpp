#include "WebServer.h"
#include "Utils.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>

WebServer::WebServer(int port = 8888)
{
    bzero(Server_Addr, sizeof(*Server_Addr));
    this->Server_Addr->sin_family = AF_INET;
    this->Server_Addr->sin_port = htons(port);
    this->Server_Addr->sin_addr->s_addr = htonl(INADDR_ANY);
    this->Listen_Fd = socket(PF_INET, SOCK_STREAM, 0);

    // 允许重用本地地址和端口(结束连接后可以立即重启)
    int flag = 1;
    setsockopt(Listen_Fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    if (bind(this->Listen_Fd, Server_Addr, typeof(Server_Addr)) < 0)
    {
        std:cout<<"Failed to bind server address"<<std::endl;
        throw std::exception();
    }
}

WebServer::~WebServer(){}

WebServer::Init()
{
    listen(this->Listen_Fd, 5);

    this->Epoll_Fd = epoll_create(5);   // size参数并不起作用，只是给内核一个提示，告诉它事件表需要多大
    AddFd(Epoll_Fd, Listen_Fd);

    // 启动线程池
    this->threadPool = new ThreadPool;
}

WebServer::EventLoop()
{
    while (Server_IsOn)
    {
        int readyEventNums = epoll_wait(this->Epoll_Fd, this->Ready_Events, MAX_EVENTS_NUMBER, timeout) // Reactor模式. 一直阻塞直到有事件来临或满足超时条件
        if (readyEventNums < 0 && errno != EINTR)   // ? errno != EINTR
        {
            // LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (auto event:Ready_Events)
        {
            // 依次取出就绪I/O事件并进行处理
            int sockFd = event.data.fd;

            // 此处可以优化
            // 有新连接
            if (sockFd == this->Listen_Fd)      
            {
                
            }
            else
            {
                if (sockFd & EPOLLIN)  // 此时与sockFd关联的文件可读
                {
                    DealReadEvent();   // 对可读文件进行处理
                }

                if (sockFd & EPOLLOUT)  // 此时与sockFd关联的文件可写
                {

                }

                if (sockFd & EPOLLOUT)  // 此时与sockFd关联的文件可写
                {

                }

                if (sockFd & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))  // 此时与sockFd关联文件发生错误，挂断或半挂断
                {
                    
                }
            }
            

            
        }
    } 
}

WebServer::DealReadEvent()      // 对可读文件进行处理：加入到请求队列中分配线程进行处理
{
    
    (*threadPool).append()
}
