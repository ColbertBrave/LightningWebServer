#ifndef LOGGING_H
#define LOGGING_H
#include <vector>
#include "LogBuffer.h"
/*
    Logging类: 一个日志线程负责收集日志信息，并将日志信息写入本地文件
    由两块缓冲区组成，一块写完后，接着写另一块，写完的那一块将日志写入本地文件后清空
    如果第二块缓冲也写完了，第一块尚未完全保存至本地文件，则开辟第三块缓冲区。
    当第一块清空后如果检测后已分配新的内存则释放掉。
*/

class Logging
{
private:
    LogBuffer Curr_Log_Buf;
    LogBuffer Next_Log_Buf;
    vector<LogBuffer> Filled_Log_Buf;



public:

    Logging(/* args */);
    ~Logging();
};




#endif