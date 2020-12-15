#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <memory>
#include <functional>

#include "HttpRequest.h"

class EventLoop;

HttpRequest::HttpRequest(): EventLoopPtr(std::make_shared<EventLoop>()) 
{}   // 在epoll里用到了

HttpRequest::HttpRequest(int socketFd, const sockaddr_in &address): Fd(socketFd), ClientAddr(address),
                                                                    EventloopPtr(std::make_shared<EventLoop>())
{
    if (HttpRequest::Request_Nums > MAX_REQUESTS)
    {
        HandleErrorEvent(503, "Too many requests now, please wait for a moment");
        CloseHttp();
    }

    HttpRequest::Request_Nums++;
    // 默认写入自带的函数对象，在新连接中会覆盖这一请求
    // 考虑通过继承关系，将EPOLLIN的状态的处理函数设置virtual
    // NewRequest类继承自HttpRequest类然后将其覆盖
    // 第一次std::bind报错，原因是加了括号: std::bind(&HttpRequest::HandleReadEvent(), this)
    // 正确的做法应当是: std::bind(&HttpRequest::HandleReadEvent, this)
    SetReadHandler(std::bind(&HttpRequest::HandleReadEvent, this));
    SetWriteHandler(std::bind(&HttpRequest::HandleWriteEvent, this));
    SetUpdateHandler(std::bind(&HttpRequest::UpdateConnect, this));
}

// TODO 定义拷贝构造函数和拷贝赋值运算符
HttpRequest::~HttpRequest()
{
    // 每销毁一个新的连接，则数量-1
    // TODO ？？放在这里好还是放在closeHttp()好。对象的析构发生在何时(准确时间)？是否可以人为控制对象的析构
    HttpRequest::Request_Nums--;
}

void HttpRequest::HandleRequest()
{
    // 根据报文做出响应，先用if-else实现
    // TODO 此处使用策略模式优化
    // TODO 此处参考linya，尚有不理解之处
    
    // 对端挂断，则关闭连接
    if ((revents & EPOLLHUP) && !(revents & EPOLLIN))
    {
        CloseHttp();
        return;
    }

    // 出错
    if (revents & EPOLLERR)
    {
        // TODO待修改，为什么linya让events=0 DONE linya每次处理完就让events=0, 是一个处理状态的flag
        // 处理完后为0，在UpdateConnect()可以对events是否为0进行检测，如果为0则视情形更新连接
        if (ErrorHandler)  
        {
             ErrorHandler();
        }
        return;
    }

    // 可写
    if (revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        ReadHandler();
    }

    // 可读
    if (revents & EPOLLOUT)
    {
        WriteHandler();
    }

    // 请求处理完后继续监听连接
    UpdateHandler();
}

void HttpRequest::HandleReadEvent()
{
    // 如果未连接，返回
    if (HttpStatus == HTTPSTATUS::DISCONNECTED)
    {
        return;
    }

    // 从fd的缓冲区循环读取数据至ReadBuffer中
    ssize_t sumReadBytes = Read(this->Fd, this->ReadBuffer);
    // 如果出错，将出错提示信息发送至客户端
    if (sumReadBytes == -1)
    {
        HandleErrorEvent(400, "Failed to read data from the client");
        return;
    }

    // 未出错，但是读不到数据，可能是对端关闭、数据未到达等原因
    if (sumReadBytes == 0)
    {
        LOG << "Read nothing\n";
        return;
    }

    // 解析读取到的数据: 解析出请求方法，URL，请求头部等HTTP信息
    Parse();

    // 根据解析出的具体请求做出响应 TODO 考虑封装成HttpResponse类
    if (ProcessStatus == PROCESSFLAG::SUCCESS)
    {
        Response();
    }

    // 完成一次响应后，重载当前HttpRequest的属性
    this->Reset();

    // 如果Response()之后，WriteBuffer中被写入了数据，那么将其发送出去
    if ((!WriteBuffer.empty()) && (ProcessStatus == PROCESSFLAG::SUCCESS))
    {
        HandleWriteEvent();
    }
}

void HttpRequest::HandleWriteEvent()
{
    if (HttpStatus == HTTPSTATUS::DISCONNECTED)
    {
        return;
    }

    ssize_t sumWriteBytes = Write(this->Fd, WriteBuffer);
    if (sumWriteBytes < 0)  // 出现异常情况
    {
        LOG << "Failed to write data to buffer\n";
        HandleErrorEvent(500, "An error occured in the server");
        return;
    }

    // WriteBuffer不为空，说明写了数据，但是没写完，将事件继续置为EPOLLOUT
    if (!WriteBuffer.empty())
    {
        this->EventPtr->events = EPOLLOUT;
    }
}

// 响应出现错误时，发送出错提示信息到客户端
void HttpRequest::HandleErrorEvent(int error_num, std::string error_msg)
{
    // 错误响应的响应行
    std::string ErrorResponse = "HTTP/1.1 " + std::to_string(error_num) + "\r\n";
    
    // 增加错误响应的响应头部信息
    ErrorResponse += "Content-Type: text/html\r\n";
    ErrorResponse += "Connection: Close\r\n";
    ErrorResponse += "Server: LightningWebServer\r\n";
    ErrorResponse += "\r\n";
    
    // 增加错误响应的主体部分
    ErrorResponse += "<html><title>哎~出错了</title>";
    ErrorResponse += "<body bgcolor=\"ffffff\">";
    ErrorResponse += std::to_string(error_num) + error_msg;
    ErrorResponse += "<hr><em>LightningWebServer</em>\n</body></html>";
    ErrorResponse += "\r\n";

    // 将错误响应信息发送至相应的客户端
    Write(Fd, ErrorResponse);
}

// 响应完请求后重新将请求更新放入EventLoop，和关联的定时器解绑，重新绑定新的定时器，属性重置
// 定时器队列是小根堆，不支持随机删除，因此先将定时器节点和request解绑，然后将其置为EXPIRED，
// 直到该定时器节点前面的定时器都被删除，轮到它时它才会被删除
void HttpRequest::UpdateConnect()
{
    DetachTimerNode();
    // 如果http连接是keep-alive的
    if ((RequestHeader.find("Connection") != RequestHeader.end()) && (RequestHeader["Connection"] != "close"))
    {
        SetEvent(EPOLLIN | EPOLLET);
        EventloopPtr->AddRequest(shared_from_this()); // TODO 是否shared_from_this //DONE 需要
    }
}

void HttpRequest::CloseHttp()
{
    HttpStatus = HTTPSTATUS::DISCONNECTED;
    // 从epoll内核事件表中移除该连接
    EventloopPtr->DeleteRequest(shared_from_this());
    close(this->Fd);
}

// 从接收缓冲区中解析出请求方法，URL，http协议版本，请求头部和请求实体
void HttpRequest::Parse()
{
    if (ReadBuffer.empty())
    {
        LOG << "The receved buffer is empty\n";
        ProcessStatus = PROCESSFLAG::PARSE_ERROR;
    }

    // 将请求报文分割为三部分: 请求行，请求头部，请求实体
    std::string requestLine = ReadBuffer.substr(0, ReadBuffer.find("\r\n"));
    std::string requestHead = ReadBuffer.substr(ReadBuffer.find("\r\n") + 2, ReadBuffer.find_last_of("\r\n"));
    std::string requestBody = ReadBuffer.substr(ReadBuffer.find_last_of("\r\n"));

    // 从请求行中解析出请求方法，URL，Http协议版本
    ParseRequestLine(requestLine);

    // 从请求头部中解析出各种头部信息，并以key-value的形式存放在map中
    ParseRequestHead(requestHead);
}

// 给定请求行，从中解析出请求方法，URL和Http版本
void HttpRequest::ParseRequestLine(std::string &requestLine)
{
    // 解析请求方法: 利用map从string映射到METHOD枚举体
    std::string requestMethodStr = requestLine.substr(0, requestLine.find(' '));
    std::map<std::string, METHOD> stringToMETHOD = {{"GET", METHOD::GET},
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
        this->ProcessStatus = PROCESSFLAG::PARSE_ERROR;
        HandleErrorEvent(400, "The request method from client is invalid");
        return;
    }
    this->RequestMethod = it->second;

    // 解析URL
    this->URL = requestLine.substr(requestMethodStr.length() + 1, requestLine.find_last_of(' ') - 1);
    if (URL.empty())
    {
        LOG << "URL is empty\n";
        ProcessStatus = PROCESSFLAG::PARSE_ERROR;
        HandleErrorEvent(400, "The URL is empty");
        return;
    }

    // 解析http版本, 目前支持HTTP 1.1
    this->HttpVersion = requestLine.substr(requestLine.find_last_of(' ') + 1, requestLine.find_last_of("\r\n") - 1);
    ProcessStatus = PROCESSFLAG::SUCCESS;
}

// 从请求头部中解析出各种头部信息，存入key-value构成的map容器中
void HttpRequest::ParseRequestHead(std::string &requestHead)
{
    while (true)
    {
        if (requestHead.size() == 0)
        {
            return;
        }
        std::string Line = requestHead.substr(0, requestHead.find("\r\n"));
        RequestHeader[Line.substr(0, Line.find(":"))] = Line.substr(Line.find(":") + 1, Line.find("\r\n"));
        // 去除已经存入map中的key-value
        requestHead = requestHead.substr(requestHead.find("\r\n") + 2);
    }
}

// 根据不同的请求方法做出对应的响应(暂时只支持GET和HEAD方法)
void HttpRequest::Response()
{
    if ((RequestMethod == METHOD::GET) || (RequestMethod == METHOD::HEAD))
    {
        // 按照响应行，响应头部，响应主体依次添加至responseMsg
        std::string responseMsg = "HTTP/1.1 200 OK\r\n";
        // 如果存在"Connection"(只有close和keep-alive两种value)，且不为close时
        // 在http/1.1中，默认是keep-alive
        // 所有的keep-alive连接默认存活DEFAULT_KEEP_ALIVE_TIME时间
        if ((RequestHeader.find("Connection") != RequestHeader.end()) && (RequestHeader["Connection"] != "close"))
        {
            responseMsg += std::string("Connection: Keep-Alive\r\n") + std::string("Keep-Alive: timeout=")
                        + std::to_string(DEFAULT_KEEP_ALIVE_TIME) + std::string("\r\n");
        }

        // 从URL中解析出GET所请求的文件路径和文件类型
        // 解析出文件路径: 先从中分割出"//"到";"的子串，然后从字串中解析出文件路径
        // 例如: http://www.mywebsite.com/sj/test;id=8079?name=sviergn&x=true#stuff
        int pos1 = URL.find("//");
        
        if (pos1 < 0)
        {
            pos1 = 0; // 如果URL缺少scheme
        }
        else
        {
            pos1 = pos1 + 2;
        }

        int pos2 = (this->URL).find(";");
        if (pos2 < 0)
        {
            pos2 = URL.size(); // // 如果URL缺少参数
        }
        std::string child_URL = URL.substr(pos1, pos2);
        if (URL.find('.') >= 0)
        {
            FilePath = child_URL.substr(child_URL.find_first_of('/') + 1);
        }
        else // URL为sj/test这种情况
        {
            FilePath = child_URL;
        }

        // 解析文件类型
        int dot_position = FilePath.find('.');
        std::string fileType;
        if (dot_position < 0) // 如果找不到文件类型前的‘.’，则文件类型为‘File’
        {
            fileType = "File";
        }
        fileType = FilePath.substr(dot_position + 1);

        // 获取GET所请求文件的文件信息
        struct stat fileStatusBuf;
        if (stat(FilePath.c_str(), &fileStatusBuf) < 0) // 异常处理
        {
            ProcessStatus = PROCESSFLAG::RESPONSE_ERROR;
            HandleErrorEvent(400, "Failed to get the required resource");
            return;
        }

        responseMsg += "Conteng-Type: " + fileType + "\r\n";
        responseMsg += "Server: Lightning Web Server\r\n";
        responseMsg += "\r\n";

        // 如果是HEAD请求方法，则将头部信息写入发送缓冲区
        if (RequestMethod == METHOD::HEAD)
        {
            WriteBuffer += responseMsg;    
            return;
        }

        // 如果是GET方法，需要将请求的资源一并写入发送缓冲区
        int sourceFd = open(FilePath.c_str(), O_RDONLY);
        if (sourceFd == -1) // 打开失败则返回-1
        {
            // 打开失败，将出错提示信息发送至客户端后返回
            ProcessStatus = PROCESSFLAG::RESPONSE_ERROR;
            HandleErrorEvent(400, "Failed to open the required resource");
            return;
        }

        // 将该文件映射至一段连续的内存
        // mmap可以把对文件的操作转为对内存的操作，以此避免更多的lseek()、read()、write()等系统调用，
        // 这点对于大文件或者频繁访问的文件尤其有用，提高了I/O效率。
        void *ret = mmap(NULL, fileStatusBuf.st_size, PROT_READ, MAP_PRIVATE, sourceFd, 0);
        if (ret == (void *) -1)
        {
            // 映射失败，将出错提示信息发送至客户端后返回
            ProcessStatus = PROCESSFLAG::RESPONSE_ERROR;
            HandleErrorEvent(400, "Failed to get the required resource");
            return;
        }

        auto firstAddr = static_cast<char*>(ret);
        responseMsg += std::string(firstAddr, firstAddr + fileStatusBuf.st_size);
        WriteBuffer += responseMsg;
        ProcessStatus = PROCESSFLAG::SUCCESS;
    }
}

void HttpRequest::SetFd(int fd)
{
    this->Fd = fd;
}

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

void HttpRequest::SetUpdateHandler(std::function<void()> &&handler)
{
    this->UpdateHandler = handler;
}

void HttpRequest::SetErrorHandler(std::function<void()> &&handler)
{
    this->ErrorHandler = handler;
}

void HttpRequest::SetEventloopPtr(std::shared_ptr<EventLoop> eventloopPtr)
{
    this->EventloopPtr = eventloopPtr;
}

// TODO 检查是否所有属性都被重置
void HttpRequest::Reset()
{
    ReadBuffer.clear();
    WriteBuffer.clear();
    RequestHeader.clear();
    URL.clear();
    HttpVersion.clear();
    FilePath.clear();
}

// 分离TimerNode和HttpRequest，reset两个对象的指针
void HttpRequest::DetachTimerNode()
{
    // 如果TimerNodeWPtr指向TimerNode的shared_ptr数量不为0
    // 返回一个指向该TimerNode的shared_ptr
    if (auto nodePtr = TimerNodeWPtr.lock())
    {   
        nodePtr->DetachHttpRequest(); // TimerNode里断开指针
        TimerNodeWPtr.reset();        // HttpRequest里断开指针 
    }
    /*
        如果TimerNodeWPtr指向TimerNode的shared_ptr数量为0
        那说明TimerNode已经和request断开，TimerNodeWPtr是一个弱指针，有TimerQueue里的shared_ptr初始化
        断开的TimerNode在小根堆里依次被销毁后，TimerNodeWPtr指向的对象也被释放。
    */
}