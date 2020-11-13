#include "HttpRequest.h"
#include <sys/epoll.h>
#include <iostream>

HttpRequest::HttpRequest() {}

// 对象构造时即将http连接添加至epollfd
HttpRequest::HttpRequest(int epollFd, int socketFd, const sockaddr_in &address)
{
    // 每创建一个新的连接，则数量+1
    if (HttpRequest::Request_Nums > ThreadPool::Max_Requests)
        HttpRequest::Request_Nums++;

    // 将链接添加至epollFd进行管理???此处是否有必要，如果仅仅是连接而无事件
    this->Socket_Fd = socketFd;
    this->Client_Addr = address;
    epoll_event httpEvent;      // epoll_event对象包含事件和数据两个成员
    httpEvent.event = EPOLLIN | EPOLLOUT;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, socketFd, httpEvent) != 0)
    {
        std::cout << "Failed to add http request into epoll event" << std::endl;
        throw std::exception();
    }
}

HttpRequest::HttpRequest(int socketFd, const sockaddr_in &address) 
{
    if (HttpRequest::Request_Nums > ThreadPool::Max_Requests)
        HttpRequest::Request_Nums++;

    this->Socket_Fd = socketFd;
    this->Client_Addr = address;
}

// TODO 定义拷贝构造函数和拷贝赋值运算符
HttpRequest::~HttpRequest()
{
    // 每销毁一个新的连接，则数量-1         
    // TODO ？？放在这里好还是放在closeHttp()好。对象的析构发生在何时(准确时间)？是否可以人为控制对象的析构
    HttpRequest::Request_Nums--;
}

bool HttpRequest::HandleRequest()
{
    // 首先解析http报文
    parse();

    // 根据报文做出响应
    // TODO 此处使用策略模式优化

    if (sockFd & EPOLLIN)  // 此时与sockFd关联的文件可读
    {
        ReadHandler();   // 对可读文件进行处理
    }

    if (sockFd & EPOLLOUT)  // 此时与sockFd关联的文件可写
    {
        WriteHandler();
    }

    if (sockFd & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))  // 此时与sockFd关联文件发生错误，挂断或半挂断
    {
        ErrorHandler(); 
    }
}

/*
    功能: 解析接收到的请求报文
    从接收缓冲区中解析出请求方法，URL，http协议版本，请求头部和请求实体
*/
void HttpRequest::parse(std::string *Recv_Buffer)
{   
    if (Recv_Buffer->size() == 0)
    {
        std::cout << "The receved buffer is empty" << std::endl;
        throw std::exception();
    }

    // 将请求报文分割为三部分: 请求行，请求头部，请求实体 
    std::string httpRequestPost = *Recv_Buffer;
    std::string requestLine = httpRequestPost.substr(0, httpRequestPost.find("\r\n"));
    std::string requestHead = httpRequestPost.substr(httpRequestPost.find("\r\n") + 2, httpRequestPost.find_last_of('\r\n'));
    std::string requestBody = httpRequestPost.substr(httpRequestPost.find_last_of('\r\n'));
    
    // 从请求行中解析出请求方法，URL，Http协议版本
    this->Request_Method = requestLine.substr(0, requestLine.find(' '));
    this->URL = requestLine.substr(Request_Method.length() + 1, requestLine.find_last_of(' ') - 1);
    this->Http_Version = requestLine.substr(requestLine.find_last_of(' ') + 1, requestLine.find_last_of('\r\n') - 1);

    // 从请求头部中解析出各种头部信息，存入key-value构成的map容器中
    while (true)
    { 
        if (requestHead.size() == 0)
        {
            break;
        }
        std::string Line = requestHead.substr(0, requestHead.find("\r\n"));
        Request_Header[Line.substr(0, Line.find(":"))] = Line.substr(Line.find(":") + 1, Line.find("\r\n"));
        // 去除已经存入map中的key-value
        requestHead = requestHead.substr(requestHead.find("\r\n") + 2);
    }

    // 从请求主体中解析相应的信息
    // 待完成
}



void HttpRequest::closeHttp()
{
    RemoveFd(HttpRequest::Epoll_Fd, this->Socket_Fd);   // 从epoll内核事件表中移除该连接
}


void HttpRequest::SetFd(int fd)
{
    this->Fd = fd;
}

// 设置epoll_event的事件类型
void HttpRequest::SetEvent(uint32_t event)
{
    this->EventPtr->events = event;
}

void HttpRequest::SetReadHandler(std::function<void()> &&handler)    //使用右值引用
{
    this->ReadHandler = handler;
}

void HttpRequest::SetWriteHandler(std::function<void()> &&handler)
{
    this->WriteHandler = handler;
}

void SetConnHandler(std::function<void()> handler)
{
    this->ConnHandler = handler;
}