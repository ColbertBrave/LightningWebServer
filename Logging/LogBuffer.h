#ifndef LOGBUGGER_H
#define LOGBUGGER_H
#include <memory>
#include <algorithm>

/*
    日志缓冲类
    定义一块日志缓冲区，一条日志消息应该单独占一行
    向缓冲区中添加消息，清空缓冲区
    获取缓冲区的已用空间，可用空间
    采用动态数组不采用vector的原因：性能更好(数组>动态数组>vector)
*/
class LogBuffer
{
private:
    size_t BufferSize;              // 缓冲区所占字节数
    std::shared_ptr<char> Buffer;   // 缓冲区的首地址
    std::shar ed_ptr<char> Cur;      // 缓冲区剩余空间的首地址         

public: 
    LogBuffer(const int size);
    ~LogBuffer();

    bool Append(char *log, size_t len);
    void Reset();
    size_t Occupied();
    size_t Available();
};


#endif