#include "HttpRequest.h"
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <functional>

HttpRequest::HttpRequest() {}

// // 对象构造时即将http连接添加至epollfd
// HttpRequest::HttpRequest(int epollFd, int socketFd, const sockaddr_in &address)
// {
//     // 每创建一个新的连接，则数量+1
//     if (HttpRequest::Request_Nums > ThreadPool::Max_Requests)
//         HttpRequest::Request_Nums++;

//     // 将链接添加至epollFd进行管理???此处是否有必要，如果仅仅是连接而无事件
//     this->Socket_Fd = socketFd;
//     this->Client_Addr = address;
//     epoll_event httpEvent;      // epoll_event对象包含事件和数据两个成员
//     httpEvent.event = EPOLLIN | EPOLLOUT;
//     if (epoll_ctl(epollFd, EPOLL_CTL_ADD, socketFd, httpEvent) != 0)
//     {
//         std::cout << "Failed to add http request into epoll event" << std::endl;
//         throw std::exception();
//     }
// }

HttpRequest::HttpRequest(int socketFd, const sockaddr_in &address)
{
    if (HttpRequest::Request_Nums > ThreadPool::Max_Requests)
        HttpRequest::Request_Nums++;

    this->Socket_Fd = socketFd;
    this->Client_Addr = address;
    // 默认写入自带的函数对象，在新连接中会覆盖这一请求
    // 考虑通过继承关系，将EPOLLIN的状态的处理函数设置virtual
    // NewRequest类继承自HttpRequest类然后将其覆盖
    SetReadHandler(std::bind(&HttpRequest::HandleReadEvent(), this));
    SetWriteHandler(std::bind(&HttpRequest::HandleWriteEvent), this);
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
    // 根据报文做出响应，先用if-else实现
    // TODO 此处使用策略模式优化
    // TODO 此处参考linya，尚有不理解之处
    if ((EventPtr->events & EPOLLHUP) && !(EventPtr->events & EPOLLIN))
    {
        return;
    }

    if (EventPtr->events & EPOLLERR)
    {
        if (errorHandler_)  //  TODO 待修改
            ErrorHandler();
        return;
    }

    if (EventPtr->events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        ReadHandler();
    }

    if (EventPtr->events & EPOLLOUT)
    {
        WriteHandler();
    }

    ConnHandler();
}

void HttpRequest::CloseHttp()
{
    RemoveFd(HttpRequest::Epoll_Fd, this->Socket_Fd); // 从epoll内核事件表中移除该连接
}

/*
    HttpRequest中并不具有各种情形下的处理方法，这些处理方法需要从外界传入
    服务器ListenFd监听接收到的新连接的处理方法在WebServer中传入
    新的连接NewConnFd上监听到请求处理方法在EventLoop中传入
*/
void HttpRequest::HandleReadEvent()
{
    if (HttpStatus == HTTPSTATUS::DISCONNECTED)
    {
        return;
    }

    ssize_t sumReadBytes = ReadToBuffer(this->Fd, this->ReadBuffer);
    if (sumReadBytes == -1)
    {
        HandleErrorEvent();
        return;
    }

    if (sumReadBytes == 0)
    {
        // 报错...
        return;
    }

    Parse();
}

void HttpRequest::HandleWriteEvent()
{
    if (HttpStatus == HTTPSTATUS::DISCONNECTED)
    {
        return;
    }

    ssize_t sumWriteBytes = WriteToBuffer(this->Fd, WriteBuffer);
    if (sumWriteBytes < 0)  // 出现异常情况
    {
        LOG << "Failed to write data to buffer\n";
        //...
    }

    // WriteBuffer不为空，说明有数据写入了，但是出错了
    if (!WriteBuffer.empty())
    {
        this->EventPtr->events = EPOLLOUT;
    }
}

// 处理异常事件
void HttpRequest::HandleErrorEvent()
{
    HttpRequest *theHttpConn = httpConnQueue[sockFd];
    theHttpConn->close();
}

void HttpRequest::SetFd(int fd)
{
    this->Fd = fd;
}


/*
    功能: 解析接收到的请求报文
    从接收缓冲区中解析出请求方法，URL，http协议版本，请求头部和请求实体
*/
void HttpRequest::Parse()
{
    if (ReadBuffer.empty())
    {
        std::cout << "The receved buffer is empty" << std::endl;
        throw std::exception();
    }

    // 将请求报文分割为三部分: 请求行，请求头部，请求实体
    std::string requestLine = ReadBuffer.substr(0, ReadBuffer.find("\r\n"));
    std::string requestHead = ReadBuffer.substr(ReadBuffer.find("\r\n") + 2, ReadBuffer.find_last_of('\r\n'));
    std::string requestBody = ReadBuffer.substr(ReadBuffer.find_last_of('\r\n'));

    // 从请求行中解析出请求方法，URL，Http协议版本
    ParseRequestLine(requestLine);

    // 从请求头部中解析出各种头部信息，并以key-value的形式存放在map中
    ParseRequestHead(requestHead);

    // 根据请求做出响应 TODO 考虑封装成HttpResponse类
    Response();

    // 如果Response()之后，WriteBuffer中被写入了数据，那么就发送出去
    if (!WriteBuffer.empty())
    {
        HandleWriteEvent();
    }

    //... TODO 还有其他异常也需要考虑
}

// 给定请求行，从中解析出请求方法，URL和Http版本
void ParseRequestLine(std::string &requestline)
{
    // 解析请求方法: 利用map从string映射到METHOD枚举体
    std::string requestMethodStr = requestLine.substr(0, requestLine.find(' '));
    map<std::string, METHOD> stringToMETHOD = {{"GET", METHOD::GET},
                                               {"POST", METHOD::POST},
                                               {"OPTIONS", METHOD::OPTIONS},
                                               {"HEAD", METHOD::HEAD},
                                               {"PUT", METHOD::PUT},
                                               {"TRACE", METHOD::TRACE},
                                               {"DELETE", METHOD::DELETE}};
    auto it = stringToMETHOD.find(requestMethodStr);
    if (it == stringToMETHOD.end())
    {
        // 当从报文中解析出的字符串无法与现有请求方法匹配时
        LOG << "The request method from client is invalid\n";
        // ... 报错 TODO
    }
    this->RequestMethod = it->second;

    // 解析URL
    this->URL = requestLine.substr(requestMethodStr.length() + 1, requestLine.find_last_of(' ') - 1);
    if (URL.empty())
    {
        LOG << "URL is empty\n";
        // ... 报错 TODO
    }

    // 解析http版本
    this->Http_Version = requestLine.substr(requestLine.find_last_of(' ') + 1, requestLine.find_last_of('\r\n') - 1);
    // ... 选择一个版本进行支持 TODO
}

// 从请求头部中解析出各种头部信息，存入key-value构成的map容器中
void ParseRequestHead(std::string &requestHead)
{
    while (true)
    {
        if (requestHead.size() == 0)
        {
            break;
        }
        std::string Line = requestHead.substr(0, requestHead.find("\r\n"));
        RequestHeader[Line.substr(0, Line.find(":"))] = Line.substr(Line.find(":") + 1, Line.find("\r\n"));
        // 去除已经存入map中的key-value
        requestHead = requestHead.substr(requestHead.find("\r\n") + 2);
    }
}

// 根据不同的请求方法做出对应的响应
void HttpRequest::Response()
{
    if (RequestMethod == METHOD::GET) || (RequestMethod == METHOD::HEAD)
    {
        // 按照响应行，响应头部，响应主体依次添加至responseMsg
        std::string responseMsg = "HTTP/1.1 200 OK\r\n";
        // 如果存在"Connection"(只有close和keep-alive两种value)，且不为close时
        // 在http/1.1中，默认是keep-alive
        // 所有的keep-alive连接默认存活DEFAULT_KEEP_ALIVE_TIME时间
        if (RequestHeader.find("Connection") != RequestHeader.end()) && (RequestHeader["Connection"] != "close") 
        {
            responseMsg += "Connection: Keep-Alive\r\n" + "Keep-Alive: timeout=" + std::to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
        }

        //...TODO 从URL中解析出GET所请求的文件路径和文件类型

        // 获取GET所请求文件的文件信息
        struct stat fileStatusBuf;
        if (stat(FilePath.c_str(), &fileStatusBuf) < 0) // 异常处理
        {
            HandleErrorEvent();
            return;
        }

        responseMsg += "Conteng-Type: " + "\r\n";
        responseMsg += "Server: Lightning Web Server\r\n";
        responseMsg += "\r\n";

        // 如果是HEAD请求方法，则返回
        if (RequestMethod == METHOD::HEAD)
        {
            return;
        }

        // 如果是GET方法，需要将请求的资源一并返回
        int sourceFd = open(FilePath.c_str(), O_RDONLY);
        if (sourceFd == -1) // 打开失败则返回-1
        {
            //... TODO
            ErrorHandler();
        }

        void *ret = mmap(NULL, fileStatusBuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
        if (ret == (void *) -1)
        {
            //...异常处理
        }
        auto firstAddr = static_cast<char *>(ret)
        responseMsg += std::string(firstAddr, firstAddr + fileStatusBuf.st_size);
        WriteBuffer += responseMsg;    
    }
}

// 设置epoll_event的事件类型
void HttpRequest::SetEvent(uint32_t event)
{
    this->EventPtr->events = event;
}

void HttpRequest::SetReadHandler(std::function<void()> &&handler) // 使用右值引用
{
    this->ReadHandler = handler;
}

void HttpRequest::SetWriteHandler(std::function<void()> &&handler)
{
    this->WriteHandler = handler;
}

void HttpRequest::SetConnHandler(std::function<void()> handler)
{
    this->ConnHandler = handler;
}