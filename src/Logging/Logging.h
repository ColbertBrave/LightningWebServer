#ifndef LOGGING_H
#define LOGGING_H
#include <pthread.h>

#include <memory>
#include <vector>
#include <string>
#include <iostream>

#include "LogBuffer.h"
/*
    Logging类: 一个日志线程负责收集日志信息，并将日志信息写入本地文件
    日志的写入接口必须是可重入的，多个其他线程可能在同一时间通过这个接口写入消息
    类构造时即创建一个线程，传入函数中开始循环接收其他线程传递的日志消息，
    当服务器关闭时，日志线程也随之关闭。
    由多块缓冲区组成一个双向循环链表，一端写入日志，另一端将日志保存至队列中写入磁盘
    TODO 将日志分为多个级别debug/error/trace
*/
const int LogBufferSize =  16 * 1024 ;         // 16KB
typedef LogBuffer<LogBufferSize> Buffer;             // NOTE 这里如何优化，用enum？还是constxpr

class Logging
{
private:
    // TODO 是否有必要将读取点对称设置在循环的另一侧 DONE 没有必要，写入和保存速度不一致，总会产生交错
    pthread_t                   LogThread_ID;
    static std::string          LogSavePath;
    static pthread_once_t       Once;
    std::shared_ptr<Buffer>     Head;           // 双向循环链表的头节点
    std::shared_ptr<Buffer>     Tail;           // 双向循环链表的尾节点
    std::shared_ptr<Buffer>     WritePtr;       // 写入指针
    std::shared_ptr<Buffer>     SavePtr;        // 保存指针
    unsigned int                LogBufferNum;
    
    std::string GenerateFileName();
    void LogThreadFunc();           // NOTE 构造函数需要用到的函数放在构造函数之前声明???
    static void* run(void* args);
    void AppendLog(std::string log);
    pthread_mutex_t Mutex;

public:
    Logging(int logBufferNum = 16);
    ~Logging();
    //void Exit(); TODO
    
    // 将"<<"运算符重载: 通过"<<"向日志缓冲区里写入日志
    // 当日志消息为std::string时
    Logging& operator<<(const std::string log)
    {
        this->AppendLog(log);
        return *this;
    }

    // 当日志消息为char类型时
    Logging& operator<<(const char log)
    {
        std::string str(1, log);
        this->AppendLog(str);
        return *this;
    }

    // 当日志消息为char数组时
    Logging& operator<<(const char log[])
    {
        std::cout << "进入了operator<<" << std::endl;
        std::string str(log);
        this->AppendLog(str);
        return *this;
    }

    void Print()
    {
        std::cout << "This is a test message" << std::endl;
    }

    // 当日志消息为数值类型时
    // 支持的数值类型取决于std::to_string支持的类型
    // 支持int/long/long long/unsigned/unsigned long/unsigned long long/float/double/long double
    template<typename T>
    Logging& operator<<(const T num)
    {
        this->AppendLog(std::to_string(num));
        return *this;
    }

    static std::shared_ptr<Logging> LoggingPtr;
    static void Singleton(void)
    {
        std::cout << "创建Singleton" << std::endl;
        // if (!LoggingPtr)
        {
            Logging logSingleton;
            Logging::LoggingPtr = std::make_shared<Logging>(logSingleton);
        }
        std::cout << "出Singleton()" << std::endl;
    }

    // 创建目录，确定是否具有访问权限
    void SetLogSavePath(std::string logSavePath)
    {
        Logging::LogSavePath = logSavePath;
        std::cout << "已经写入了日志保存路径" << std::endl;
    }

    /*
        为什么这个函数的定义放在cpp中时会报错？
        并且如果Once不加Logging::，会提示
        Logging.cpp: In function ‘Logging& Init()’:
        Logging.cpp:137:19: error: ‘Once’ was not declared in this scope
        137 |     pthread_once(&Once, Logging::Singleton);
            |                   ^~~~
        
        Logging.cpp: In function ‘Logging Init()’:
        Logging.cpp:137:29: error: ‘pthread_once_t Logging::Once’ is private within this context
        137 |     pthread_once(&(Logging::Once), Logging::Singleton);
            |                             ^~~~
        Logging.cpp:11:16: note: declared private here
        11 | pthread_once_t Logging::Once = PTHREAD_ONCE_INIT;
            |                ^~~~~~~
        make: *** [Makefile:9: Logging.o] Error 1
    */
    static Logging& Init()
    {
        if (pthread_once(&Once, Logging::Singleton) !=0)
        {
            std::cout << "Error occured" << std::endl;
        }
        
        std::cout << "返回一个Logging对象" << std::endl;
        return *(Logging::LoggingPtr);
    }
    // {
    //     // pthread_once指定的函数仅执行一次
    //     // int pthread_once(pthread_once_t *once_control, void (*init_routine) (void));
    //     // once_control表征是否执行过
    //     Logging::LogSavePath = logSavePath;
    //     pthread_once_t Once = PTHREAD_ONCE_INIT;
    //     pthread_once(&Once, Logging::Singleton);

    //     return *LoggingPtr;
    // }

    // static Logging& Init()
    // {
    //     return *LoggingPtr;
    // }
};
// 使用了线程安全的单例模式
// do while(0)的意义是可以break
#define LOG_INIT(logSavePath)       \
    do                              \
    {                               \
        (Logging::Init()).SetLogSavePath(logSavePath); \
    } while (0)

#define LOG  Logging::Init() 
#endif