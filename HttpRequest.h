#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <map>
#include "Utils.h"


/*
    Http连接是建立在TCP连接之上的
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
    enum class TriggerMode
    {

    }

private:
    int             Socket_Fd;      // 该链接对应的socket
    sockaddr_in     Client_Addr;
    std::string     Http_Version;
    std::string     URL;
    METHOD          Request_Method;
    TriggerMode Trigger_Mode;
    std::string     *Recv_Buffer;   // 连接里的读写缓冲区的本质及其意义是什么

    map<std::string, std::string> Request_Header;
    void parse();


public:
    static int Epoll_Fd;            // 所有HTTP请求的对象都由同一个EpollFd进行管理，因此是静态数据成员，每个对象创建后均需要添加至Epoll进行管理？？？
    static unsigned int Request_Nums;
    bool process();
    bool write();
    void closeHttp();
}
unsigned int HttpRequest::Request_Nums = 0;

#endif