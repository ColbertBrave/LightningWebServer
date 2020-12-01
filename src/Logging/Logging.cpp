#include "Logging.h"
#include <iomanip>
#include <ctime>
#include <sstream>
#include <iostream>
#include <pthread.h>
#include <memory>

// 构造函数: 初始化各数据成员，建立双向缓冲区链表，建立日志线程
Logging::Logging()
{
    // 建立一个双向缓冲区链表
    int logBufferNum = 16;  // TODO 如何用默认形参代替
    // 定义双向循环缓冲区链表的头节点和尾节点, 初始时只有一个节点
    // NOTE make_shared是动态分配了一个新的对象，然后用*Tail初始化了它的各项属性，和Tail是两码事
    
    Head = std::make_shared<Buffer>();
    Tail = Head;
    logBufferNum--;
    Head->Next = Tail;
    Head->Prev = Tail;
    Tail->Next = Head;
    Tail->Prev = Head;

    while (logBufferNum > 0)
    {
        std::shared_ptr<Buffer> New = std::make_shared<Buffer>();
        New->Prev = Tail;
        New->Next = Head;
        Tail->Next = New;
        Head->Prev = New;
        Tail = New;
        logBufferNum--;
    }
    this->WritePtr = Head;
    this->SavePtr = Tail;

    if (pthread_create(&LogThread_ID, NULL, run, this) != 0)
    {
        std::cout << "Error occured when creating log thread" << std::endl;
        throw std::exception();
    }
    pthread_detach(LogThread_ID);
}

Logging::~Logging() {}

// 日志写入速度要快于持久化的速度。确保在正常退出时所有缓冲区的日志都已经持久化 TODO 有问题
// void Logging::Exit()
// {
//     std::shared_ptr<Buffer> check = std::make_shared<Buffer>(*Head);
//     for (auto i = 0; i < 16;i++)
//     {
//         if ((check->Status == Buffer::FREE) && (check->Status == Buffer::LOCK))
//         {
//             //cout << "第" << i <<"个buffer已空" << endl;
//             continue;
//         }
//         // cout << check->Status << endl;
//         // cout << Head->Status << endl;
//         // cout << Tail->Status << endl;
//         while (check->Status == Buffer::FULL) // 如果当前节点已满，则等待其保存
//         {
//             //cout << "yes" << endl;
//             if ((check->Status == Buffer::FREE) && (check->Status == Buffer::LOCK)) // 当前节点已保存
//             {
//                 check->Status = Buffer::LOCK; // 锁定当前buffer
//                 check = check->Next;
//                 break;
//             }
//         }
//         //cout << "清空了第"<< i <<"个Buffer" << endl;
//     }
// }

std::string Logging::GenerateFileName()
{
    auto time = std::time(nullptr);
    auto localTime = *std::localtime(&time);
    std::ostringstream fileNameStream;
    fileNameStream << std::put_time(&localTime, "%Y-%m-%d %H-%M.txt");
    return fileNameStream.str();
}

// 向双向缓冲区链表里写入日志
void Logging::AppendLog(std::string log)
{
    // 如果当前缓冲区已满，则选择下一个
    while (!WritePtr->Available(log, sizeof(log)))
    {
        WritePtr = WritePtr->Next;
    }
}

void Logging::LogThreadFunc()
{
    // 从缓冲区内将日志保存至本地文件
    while(WebServer::Server_Run)
    {
        // 条件变量，当FullFlag变为true时，即将缓冲区的日志保存至磁盘中
        // 保存后，清空了日志缓冲区，重置了标志位
        if (SavePtr->Status == 1) // 缓冲区已满
        {
            SavePtr->SaveToFile(this->GenerateFileName());
            //pthread_mutex_unlock(&Mutex);       // 首先释放锁，保存时间长且无需加锁 NOTE 也许有问题
        }
        SavePtr = SavePtr->Next;
    }
}

void* Logging::run(void *args)      // NOTE 在h文件中声明的静态成员函数，在cpp文件无需加上static关键字
{
    Logging *LOG = static_cast<Logging *>(args);
    if (!LOG)
    {
        throw std::exception();
    }
    LOG->LogThreadFunc();
    return NULL;
}