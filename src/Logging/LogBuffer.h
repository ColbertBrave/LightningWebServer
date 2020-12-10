#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include <sys/syscall.h>
#include <pthread.h>

#include <string>
#include <algorithm>
#include <array>
#include <fstream>
#include <memory>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <unistd.h>
#include <iostream>

#define gettid() syscall(SYS_gettid)
/*
    日志缓冲类
    定义一块日志缓冲区，一条日志消息以\r\n结束
    包含：向缓冲区中添加消息，清空缓冲区，获取缓冲区的已用空间，可用空间
    选择std::array的原因，不采用vector的原因: 
    从性能的角度来讲：数组 > 动态数组 > vector
    https://stackoverflow.com/questions/30263303/stdarray-vs-array-performance
    https://blog.csdn.net/fengbingchun/article/details/72809699
    https://www.inf.pucrs.br/~flash/progeng2/cppreference/w/cpp/container/array.html
    std::array与C的数组均属于静态数组，在大部分场景下拥有近似的性能，
    更加安全，更加方便，提供了 STL 的各种迭代器、算法、操作方法等，
    std::array不会隐式转成指针(要指针请显式调用 data())，可以方便地按值传递、按值返回、赋值。
    每一块日志缓冲区都包含一个锁，写入和保存均需要先获得互斥锁

    LogBuffer不需要在容器的两端插入或者删除元素，也不需要在中间插入和删除元素，需要随机访问元素，
    因此可以排除list/deque/forward_list，初步入选: array/vector/string
    LogBuffer的大小是固定的，多个LogBuffer构成一个循环链表。大小固定的好处是可以避免频繁的内存分配和回收，
    array/vector/string这三者都将元素保存在连续的内存空间中，其中vector/string容器的大小是可变的，
    为了减少容器中元素增加时重新分配内存的次数，通常vector/string会分配比需求更大的空间，将多分配的空间留作预备
    vector/string的reserve(n)方法至少分配与需求n一样大的空间，但也可能更大。当然也可以shrink_to_fit来要求vector/
    string/deque退回不需要的内存空间，但是具体的实现过程中可以选择忽略此请求，也就是说，调用shrinktofit也并不保证一定退回内存空间。
    [C++ Primer 第五版，P357]
    在LogBuffer中内存大小时固定的，不会发生改变，因此选用array可以兼顾各方面需求
*/
template <unsigned int SIZE>
class LogBuffer
{
private:
    size_t                          BufferSize;     // 缓冲区所占字节数
    std::array<char, SIZE>          Buffer;         // 日志缓冲区
    unsigned int                    Cur;            // 日志当前写入位置
    
public:
    LogBuffer(): BufferSize(SIZE), Prev(nullptr), Next(nullptr), Cur(0), Mutex(PTHREAD_MUTEX_INITIALIZER)
    {
        // TODO 一个默认构造的std::array已经是非空的，都被默认初始化，因此无需进行初始化。验证后更改
        this->Buffer.fill('\0');
    }
    // TODO 增加拷贝构造函数和拷贝赋值运算符
    ~LogBuffer() 
    {
        if (!pthread_mutex_lock(&Mutex))
        {
            pthread_mutex_destroy(&Mutex);
        }
    }
    
    std::shared_ptr<LogBuffer>   Prev;           // 上一块缓冲区指针   
    std::shared_ptr<LogBuffer>   Next;           // 下一块缓冲区指针
    pthread_mutex_t              Mutex;
    enum Buffer_Status
    {
        FREE,       // 可以写入
        FULL=1,     // 缓冲区已满，不能写入
        LOCK        // 锁定，用于退出，不能写入
    };
    Buffer_Status Status;

    // 向日志缓冲区写入日志
    bool Append(std::string log, size_t len)
    {
        std::cout << "进入了Append" << std::endl;
        // 判断要写入的日志大小是否小于可用空间
        // 如果空间不够，返回false，将选用下一块缓冲区写入
        // TODO flag更改的位置是否合理
        pthread_mutex_lock(&Mutex);
        if (!this->Available(len))
        {
            this->Status = FULL;
            pthread_mutex_unlock(&Mutex);
            return false;
        }

        std::cout << "在AppendLog中对buffer上锁后" << std::endl;
        // 每条日志消息都加上时间戳
        auto time = std::time(nullptr);
        auto localTime = *std::localtime(&time);
        std::ostringstream timeStream;
        timeStream << std::put_time(&localTime, "[%Y-%m-%d %H:%M:%S ]");
        std::string totalLog = timeStream.str() + "[Tid:" +std::to_string(gettid())+"] " + log;

        std::cout << "在AppendLog中开始写入日志" << std::endl;
        // 空间足够，写入日志
        for (auto i : totalLog)
        {
            Buffer[Cur] = i;
            Cur++;                      // TODO memcpy/move等其他写入方式
        }
        pthread_mutex_unlock(&Mutex);
        std::cout << "日志写入完毕" << std::endl;
        return true;
    }

    // 将日志缓冲区清0
    void Reset()
    {
        pthread_mutex_lock(&Mutex);
        this->Buffer.fill('\0');    // 没有必要调用fill, 验证后可以更改
        this->Cur = 0;
        //this->FullFlag = false;
        pthread_mutex_unlock(&Mutex);
    }

    // 返回日志缓冲区的已用空间
    size_t Occupied()
    {
        // NOTE 此处是否会出现竞态条件
        return static_cast<size_t>(this->Cur);
    }

    // 返回日志缓冲区的可用空间
    size_t Available()
    {
        return this->BufferSize - this->Occupied();
    }

    // 有日志要写入，返回空间是否足够
    bool Available(size_t logLength)
    {
        //pthread_mutex_lock(&Mutex);
        size_t avil = BufferSize - Cur;
        if (avil > logLength)
        {
            //pthread_mutex_unlock(&Mutex);
            return true;
        }
        else
        {
            this->Status = FULL;
            //this->FullFlag = true;
            //pthread_mutex_unlock(&Mutex);
            return false;
        }
    }

    // 将内存里的日志保存至磁盘里，保存后缓冲区清空
    /*
        保存的过程无需加锁：
        1 获取锁检测LogBuffer的状态，如果为FREE，释放锁，重新检测下一块缓冲区；
        2 如果FULL，释放锁，开始保存至本地文件，然后重置标志位。
        因为被置为FULL以后，即使释放了锁其他线程检测到FULL以后也不会再写入，
        这期间除了保存以外，没有其他操作，所以无需加锁。
    */
    void SaveToFile(std::string fileName)
    {
        std::cout << "In the save to file function" << std::endl;
        // 以追加模式将当前日志append到fileName文件的末尾
        std::ofstream outfile(fileName, std::ios_base::app);
        for (auto i = 0; i < this->Cur; i++)
        {
            outfile << Buffer[i];       
            // 考虑std::move, std::copy, std::swap
            // swap(a, b) 交换a和b中的元素，这样比拷贝快得多
            // 循环会造成效率的损失
        }
        this->Reset();
        outfile << std::endl;
        outfile.close();
        std::cout << "日志保存完毕" << std::endl;
    }
};

#endif