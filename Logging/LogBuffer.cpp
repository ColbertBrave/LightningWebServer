#include "LogBuffer.h"

// 使用智能指针，此处调试还有些问题没有修改
LogBuffer::LogBuffer(const int size)
{
    this->Buffer = new char[size];
    this->BufferSize = sizeof(char) * size;
    // 日志缓冲区用'0'初始化
    *Buffer = {'0'};
    this->Cur = Buffer;
}

LogBuffer::~LogBuffer()
{
    delete Buffer;
}

bool LogBuffer::Append(char *log, size_t len)
{
    // 判断要写入的日志大小是否小于可用空间
    if (this->Available() < len)
    {
        return false;
    }

    memcpy(this->Cur, log, len);
    this->Cur = this->Cur + len;
    return true;
}

size_t LogBuffer::Available()
{
    return this->BufferSize - this->Occupied();
}

size_t LogBuffer::Occupied()
{
    return (this->Cur - Buffer) * sizeof(char);
}

void LogBuffer::Reset()
{
    *Buffer = {'0'};
}
