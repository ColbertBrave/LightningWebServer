#ifndef LOGGING_H
#define LOGGING_H
#include <vector>
#include <string>
#include <pthread.h>
#include <memory>
#include "LogBuffer.h"
/*
    Logging类: 一个日志线程负责收集日志信息，并将日志信息写入本地文件
    日志的写入接口必须是可重入的，多个其他线程可能在同一时间通过这个接口写入消息
    类构造时即创建一个线程，传入函数中开始循环接收其他线程传递的日志消息，
    当服务器关闭时，日志线程也随之关闭。
    由多块缓冲区组成一个双向循环链表，一端写入日志，另一端将日志保存至队列中写入磁盘
    TODO 将日志分为多个级别debug/error/trace
*/
const int LogBufferSize =  16 * 1024 * 1024;         // 16MB
typedef LogBuffer<LogBufferSize> Buffer;             // NOTE 这里如何优化，用enum？还是constxpr
class Logging
{
private:
    // TODO 是否有必要将读取点对称设置在循环的另一侧 DONE 没有必要，写入和保存速度不一致，总会产生交错
    pthread_t                   LogThread_ID;
    std::shared_ptr<Buffer>     Head;           // 双向循环链表的头节点
    std::shared_ptr<Buffer>     Tail;           // 双向循环链表的尾节点
    std::shared_ptr<Buffer>     WritePtr;       // 写入指针
    std::shared_ptr<Buffer>     SavePtr;        // 保存指针
    std::string GenerateFileName();
    void LogThreadFunc();           // NOTE 构造函数需要用到的函数放在构造函数之前声明???
    static void* run(void* args);
    void AppendLog(std::string log);
    pthread_mutex_t Mutex;

public:
    Logging();
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
        std::string str(log);
        this->AppendLog(str);
        return *this;
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
};
#define LOG Logger
#endif