#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <string>
#include <map>
#include <memory>
#include <functional>

#include "Utils.h"
#include "Timer/Timer.h"

/*
    Http连接是建立在TCP连接之上的
    HttpRequest类并不处理请求，只负责将新的连接封装成请求，并解析请求内容
    相应的处理方法有自带处理方法，也可以由外界传入
*/
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000;  // 一个keep-alive连接的默认存活事件(ms)
const long int MAX_REQUESTS = 10000000;
std::map<std::string, std::string> SuffixToFiletype = {{".html", "text/html"},
                                                  {".avi", "video/x-msvideo"},
                                                  {".bmp", "image/bmp"},
                                                  {".c", "text/plain"},
                                                  {".jpg", "image/jpeg"},
                                                  {".png", "image/png"},
                                                  {".txt", "text/plain"}};
class EventLoop;

class HttpRequest: public std::enable_shared_from_this<HttpRequest>
{
public:
    enum class METHOD
    {
        GET, POST, HEAD, PUT, TRACE, OPTIONS, DELETE
    };
    enum class HTTPSTATUS
    {
        CONNECTED, DISCONNECTED
    };
    enum class PROCESSFLAG
    {
        PARSE_ERROR, SUCCESS, RESPONSE_ERROR
    };

private:
    std::shared_ptr<EventLoop>  EventloopPtr;
    std::function<void()>       ReadHandler;
    std::function<void()>       WriteHandler;
    std::function<void()>       ErrorHandler;
    std::function<void()>       UpdateHandler;

    // 可以封装为一个struct httpdata{}
    std::string                     FilePath;       // GET/HEAD方法请求文件所在的文件路径
    sockaddr_in                     ClientAddr;
    std::string                     HttpVersion;
    std::string                     URL;
    METHOD                          RequestMethod;
    std::map<std::string, std::string>   RequestHeader;

    // https://stackoverflow.com/questions/48704563/should-i-use-unique-ptr-for-a-string
    // 对于std::string 无需使用智能指针
    // std::shared_ptr<std::string>    ReadBuffer;   
    // std::shared_ptr<std::string>    WriteBuffer;
    // TODO 引用还是不引用好，或者指针好
    std::string ReadBuffer;
    std::string WriteBuffer;
    // TODO 连接里的读写缓冲区的本质及其意义是什么 DONE 从socketfd里读写

    // 解析请求
    void Parse();
    void ParseRequestLine(std::string& requestline);
    void ParseRequestHead(std::string& requestline);

public:
    HttpRequest();
    HttpRequest(int socketFd, const sockaddr_in& address);
    ~HttpRequest();

    int                         Fd;                 // 该连接对应的socket fd
    epoll_event*                EventPtr;           // fd对应的epoll_event, 用于epoll_ctl()添加
    uint32_t                    revents;            // 实际的事件状态
    HTTPSTATUS                  HttpStatus; // 该http连接所对应的状态
    std::weak_ptr<TimerNode>    TimerNodeWPtr;
    PROCESSFLAG                 ProcessStatus;
    // static int Epoll_Fd;
    // OLD 所有HTTP请求的对象都由同一个EpollFd进行管理，因此是静态数据成员，每个对象创建后均需要添加至Epoll进行管理？？？
    // DONE 所有请求在被接收后就封装分发至epoll监听
    static unsigned int Request_Nums;

    // 设置相关属性
    void SetFd(int fd);
    void SetEvent(uint32_t event);
    void SetReadHandler(std::function<void()> &&handler);
    void SetWriteHandler(std::function<void()> &&handler);
    void SetUpdateHandler(std::function<void()> &&handler);
    void SetErrorHandler(std::function<void()> &&handler);
    void SetEventloopPtr(std::shared_ptr<EventLoop> eventloopPtr);

    // 处理请求
    void HandleRequest();
    void HandleReadEvent();
    void HandleWriteEvent();
    void HandleErrorEvent(int error_num, std::string error_msg);
    void UpdateConnect();
    void Response();
    void Reset();
    void CloseHttp();
    void DetachTimerNode();
};

unsigned int HttpRequest::Request_Nums = 0;
#endif