#include <pthread.h>

#include <memory>
#include <sstream>
#include <iostream>
#include <ctime>
#include <iomanip>

#include "Logging.h"

pthread_once_t Logging::Once = PTHREAD_ONCE_INIT;
std::shared_ptr<Logging> Logging::LoggingPtr = nullptr;
std::string Logging::LogSavePath = "";

// 构造函数: 初始化各数据成员，建立双向缓冲区链表，建立日志线程
Logging::Logging(int logBufferNum): LogBufferNum(logBufferNum)
{
    // 建立一个双向缓冲区链表
    // int logBufferNum = 16// TODO 如何用默认形参代替
    // 定义双向循环缓冲区链表的头节点和尾节点, 初始时只有一个节点
    // NOTE make_shared是动态分配了一个新的对象，然后用*Tail初始化了它的各项属性，和Tail是两码事
    // 补充 这里其实没有建立一个双向循环链表，单向循环链表足矣
    
    Head = std::make_shared<Buffer>();
    Tail = Head;
    LogBufferNum--;
    Head->Next = Tail;
    Head->Prev = Tail;
    Tail->Next = Head;
    Tail->Prev = Head;

    while (LogBufferNum > 0)
    {
        std::shared_ptr<Buffer> New = std::make_shared<Buffer>();
        New->Prev = Tail;
        New->Next = Head;
        Tail->Next = New;
        Head->Prev = New;
        Tail = New;
        LogBufferNum--;
    }
    this->WritePtr = Head;
    this->SavePtr = Tail;

    // 纠正 改用pthread_once
    // 补充 pthread_once的好处
    // 更新 还是要用Pthread_create, 混淆了这两者之间的关系
    if (pthread_create(&LogThread_ID, NULL, run, this) != 0)
    {
        std::cout << "Error occured when creating log thread" << std::endl;
        throw std::exception();
    }
    pthread_detach(LogThread_ID);
    //pthread_once(&once, run);
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
    fileNameStream << LogSavePath <<std::put_time(&localTime, "%Y-%m-%d %H-%M.txt"); //TODO 这里的文件路径名似乎有些问题
    std::cout << fileNameStream.str() << std::endl;
    return fileNameStream.str();
}

// 向双向缓冲区链表里写入日志
void Logging::AppendLog(std::string log)
{
    // 如果当前缓冲区已满，则选择下一个
    while (!WritePtr->Append(log, sizeof(log)))
    {
        WritePtr = WritePtr->Next;
    }
}

void Logging::LogThreadFunc()
{
    // 从缓冲区内将日志保存至本地文件
    while(true)
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
    Logging *LOGPTR = static_cast<Logging *>(args);
    if (!LOGPTR)
    {
        throw std::exception();
    }
    LOGPTR->LogThreadFunc();
    return NULL;
}

// // static方法
// Logging& Init()
// {

// }