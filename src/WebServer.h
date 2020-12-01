#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <vector>
#include <memory>

#include "ThreadPool.h"
#include "HttpRequest.h"
#include "Logging/Logging.h"
#include "EventLoop.h"

#define MAX_EVENTS_NUMBER 20000
#define MAX_CONNECTIONS 65536

extern Logging LOG;

class WebServer
{
private:
    int                             ListenFd;
    std::unique_ptr<ThreadPool>     Threadpool;
    std::shared_ptr<EventLoop>      MainLoop;
    std::shared_ptr<HttpRequest>    NewRequest;

    void DistributeNewRequest();

    // vector<HttpRequest *> httpConnQueue(MAX_CONNECTIONS, 0);
    // TODO 存储就绪的事件, 一次就绪数量有限制，为什么要这么设置？是否可以设置成动态的，单个事件是否可以使用auto类型
    // DONE 没有必要设置成动态的，单个事件可以用auto，不过依然没有必要，因为auto在这里所代表的类型只可能是一种
    // 没有必要为了使用新特性而去使用，auto用于以下几种情况：
    // 1 用于代替冗长复杂、变量使用范围专一的变量声明，如std::vector<std::string>::iterator i = vs.begin()
    // 2 在定义模板函数时，用于声明依赖模板参数的变量类型，如auto v = x + y; 其中x和y都是模板参数类型
    // auto变量类型在初始化时确定，在编译时会自动推断变量类型，因此并不会造成性能的损失。
public:
    WebServer(std::shared_ptr<EventLoop> loop);
    ~WebServer();

    // 启动服务器
    void Start();

public:
    struct sockaddr_in  Server_Addr;
    static bool         Server_Run;
}

#endif