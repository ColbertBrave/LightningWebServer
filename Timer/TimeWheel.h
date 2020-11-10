#ifndef TIMEWHEEL_H
#define TIMEWHEEL_H

/*
    时间轮定时器，优于双向链表定时器
    基于双向链表的定时器使用一条链表来管理所有定时器，因此，随着定时器数目的增多，链表插入操作的效率降低
    时间轮基于哈希表，将定时器散列到不同的链表上，每条链表上的定时器数目都将明显少于原来的一条排序链表的定时器数目，
    因此插入操作的效率可以不受到定时器数量增加的影响。
*/
class TimeWheel
{
private:
    /* data */
public:
    TimeWheel(/* args */);
    ~TimeWheel();
};

TimeWheel::TimeWheel(/* args */)
{
}

TimeWheel::~TimeWheel()
{
    
}


#endif