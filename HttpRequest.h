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
private:
    int         Socket_Fd;      // 该链接对应的socket
    sockaddr_in Address;
    std::string Http_Version;
    std::string URL;
    std::string Request_Method;
    // enum class Request_Method
    // {
    //     GET,
    //     HEAD,
    //     POST,
    //     PUT,
    //     TRACE,
    //     OPTIONS,
    //     DELETE
    // };

    void parse();

public:
    static int Epoll_Fd;            // 所有HTTP请求的对象都由同一个EpollFd进行管理，因此是静态数据成员，每个对象创建后均需要添加至Epoll进行管理
    bool process();
}

#endifd