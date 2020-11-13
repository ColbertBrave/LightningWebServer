#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string>
#include <map>
#include <functional>
#include "EventLoop.h"
#include "Utils.h"


/*
    Http连接是建立在TCP连接之上的
    HttpRequest类并不处理请求，只负责将新的连接封装成请求，并解析请求内容
    相应的处理方法由外部传入。
*/
class HttpRequest
{
public:
    enum class METHOD
    {
        GET,
        POST,
        HEAD,
        PUT,
        TRACE,
        OPTIONS,
        DELETE
    };

private:
    std::make_shared<EventLoop> EventLoop
    
                    
    sockaddr_in                 Client_Addr;
    std::string                 Http_Version;
    std::string                 URL;
    METHOD                      Request_Method;
    TriggerMode                 Trigger_Mode;
    std::string                 *Recv_Buffer;   // 连接里的读写缓冲区的本质及其意义是什么

    map<std::string, std::string> Request_Header;
    void parse();

public:
    HttpRequest();
    HttpRequest(int fd, struct sockaddr_in address);
    ~HttpRequest();

    epoll_event*                EventPtr;      
    int                         Fd;                 // 该连接对应的socket fd
    // static int Epoll_Fd;            
    // OLD所有HTTP请求的对象都由同一个EpollFd进行管理，因此是静态数据成员，每个对象创建后均需要添加至Epoll进行管理？？？
    // DONE 所有请求在被接收后就封装分发至epoll监听
    static unsigned int Request_Nums;
    

    void closeHttp();

    std::function<void()> ReadHandler;
    std::function<void()> WriteHandler;
    std::function<void()> ErrorHandler;
    std::function<void()> ConnHandler;

    void SetFd(int fd);
    void SetEvent(uint32_t event);
    void SetReadHandler(std::function<void()> handler);
    void SetWriteHandler(std::function<void()> handler);
    void SetConnHandler(std::function<void()> handler);
    void HandleRequest();
}
#endif