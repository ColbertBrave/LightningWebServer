#include "TimerLinkedList.h"

TimerLinkedList::TimerLinkedList(/* args */)
{
    
}

TimerLinkedList::~TimerLinkedList()
{

}

bool TimerLinkedList::InsertTimer(Timer *timer)
{
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

bool TimerLinkedList::DeleteTimer(Timer *timer)
{
    
}