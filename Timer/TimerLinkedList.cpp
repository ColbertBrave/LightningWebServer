#include "TimerLinkedList.h"

TimerLinkedList::TimerLinkedList(/* args */)
{
    
}

TimerLinkedList::~TimerLinkedList()
{

}

// 向定时器链表中插入定时器
bool TimerLinkedList::InsertTimer(Timer *timer)
{
    // 对传入参数进行检查
    if (!timer)
    {
        throw std::exception();
        return false;
    }
    
    // 当定时器链表为空时
    if (!this->Head_Timer)
    {
        Head_Timer = timer;
        Tail_Timer = timer;
        return true;
    }

    // 当插入的定时器小于当前链表所有定时器的超时时间时
    if (timer->Expire_Time < Head_timer->Expire_Time)
    {
        timer->Pre_Timer = NULL;
        timer->Next_Timer = Head_Timer;
        Head_Timer->Pre_Timer = timer;
        this->Head_Timer = timer;
        return true;
    }
    
    // 当插入的定时器大于当前链表所有定时器的超时时间时
    if (timer->Expire_Time > Tail_Timer->Expire_Time)
    {
        timer->Pre_Timer = Tail_Timer;
        timer->Next_Timer = NULL;
        Tail_Timer->Next_Timer = timer;
        this->Tail_Timer = timer;
        return true;
    }

    // 当插入的定时器位于定时器链表中间时
    Timer &current = *Head_Timer;
    while (current)
    {
        if (timer->Expire_Time > current->Expire_Time && timer->Expire_Time < current->Next_Timer->Expire_Time) 
        {
            timer->Next_Timer = current.Next_Timer.Next_Timer;
            timer->Pre_Timer = current;
            current.Next_Timer.Pre_Timer = timer;
            current.Next_Timer = timer;
            return true;
        }
        current = current.Next_Timer;
    }
}

// 删除定时器链表中的定时器
bool TimerLinkedList::DeleteTimer(Timer *timer)
{
    // 对传入参数进行检查
    if (!timer)
    {
        throw std::exception();
        return false;
    }

    // 当该定时器为头节点时
    if (timer == Head_Timer)
    {
        this->Head_Timer = Head_Timer->Next_Timer;
        Head_Timer->Next_Timer->Pre_Timer = nullptr;
        return true;
    }

    // 当该定时器不为头节点时
    Timer &current = *(Head_Timer->Next_Timer);
    while (current)
    {
        Timer prev = Head_Timer;
        if (current == timer)
        {
            prev.Next_Timer = current.Next_Timer;
            (current.Next_Timer).Pre_Timer = prev;
            return true;
        }

        prev = current;
        current = current.Next_Timer;
    }

    // 当遍历了整个链表找不到该定时器
    std::cout << "The timer doesn't exist" << std::endl;
    return true;
}


// 清除过期的定时器
void TimerLinkedList::ClearExpired()
{
    // 检查定时器链表是否为空
    if (!Head_Timer)
    {
        std::cout << "The timer linked list is empty" << std::endl;
        return;
    }

    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    Timer temp = *Head_Timer;
    while (temp)
    {
        // 当前时间小于定时器链表所有定时器的时间
        if (currentTime < temp.Expire_Time)
        {
            std::cout << "There is no timer needed to be cleared" << endl;
            return;
        }

        // 当temp节点已经超时，则调用回调函数处理事件, 并删除这个节点
        temp.pf_Callback(); // TODO 这里的用户数据应当如何封装
        temp = temp.Next_Timer; // TODO 尚未完成
    }
}