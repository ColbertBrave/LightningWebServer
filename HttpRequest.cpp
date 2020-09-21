#include "HttpRequest.h"
#include <sys/epoll.h>
#include <iostream>
#include <string>

HttpRequest::HttpRequest() {}

// 对象构造时即将http连接添加至epollfd
HttpRequest::HttpRequest(int epollFd, int socketFd, const sockaddr_in &address) 
{
    // 将链接添加至epollFd进行管理
    this->Socket_Fd = socketFd;
    this->Address = address;
    epoll_event httpEvent;      // epoll_event对象包含事件和数据两个成员
    httpEvent.event = EPOLLIN | EPOLLOUT;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, socketFd, httpEvent) != 0)
    {
        std::cout << "Failed to add http request into epoll event" << std::endl;
        throw std::exception();
    }
}

bool HttpRequest::process()
{
    // 首先解析http报文，明确要做什么
    parse();

}

void HttpRequest::parse(std::string *Recv_Buffer)
{
    const std::string httpRequestMessage = *Recv_Buffer;
    std::string requestLine = httpRequestMessage.substr(0, httpRequestMessage.find("\r\n"));
    this->Request_Method = requestLine.substr(0, requestLine.find(' '));
    this->URL = requestLine.substr(Request_Method.length() + 1, requestLine.find_last_of(' ') - 1);
    this->Http_Version = requestLine.substr(requestLine.find_last_of(' ') + 1, requestLine.find_last_of('\r\n') - 1);
}