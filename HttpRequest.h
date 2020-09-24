#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>


/*
    Http连接是建立在TCP连接之上的
*/
class HttpRequest
{
public:
    enum class METHOD
    {
        GET,
        HEAD,
        POST,
        PUT,
        TRACE,
        OPTIONS,
        DELETE
    };

private:
    int         Socket_Fd;      // 该链接对应的socket
    sockaddr_in Client_Addr;
    std::string Http_Version;
    std::string URL;
    METHOD Request_Method;
    std::string *Recv_Buffer;   // 连接里的读写缓冲区的本质及其意义是什么



    void parse();

public:
    //static int Epoll_Fd;            // 所有HTTP请求的对象都由同一个EpollFd进行管理，因此是静态数据成员，每个对象创建后均需要添加至Epoll进行管理？？？
    static unsigned int Request_Nums;
    bool process();
} 
unsigned int HttpRequest::Request_Nums = 0;

#endif