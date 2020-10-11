#ifndef TIMERLINKEDLIST_H
#define TIMERLINKEDLIST_H
#include <time.h>

typedef struct 
{
    
} ClientData;


typedef struct
{
    Timer *Pre_Timer = NULL;
    Timer *Next_Timer = NULL;
    time_t Expire_Time;
    void (*pf_Callback)(ClientData *client_data);
} Timer;

/*
    定时器链表按照升序排列
*/
class TimerLinkedList
{
private:
    Timer *Head_Timer;
    Timer *Tail_Timer;


public:
    TimerLinkedList(/* args */);
    ~TimerLinkedList();

    bool InsertTimer(Timer *timer);
    bool DeleteTimer(Timer *timer);

};

#endif