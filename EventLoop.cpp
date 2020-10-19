#include "EventLoop.h"
#include <cassert>

EventLoop::EventLoop(/* args */)
{

}

EventLoop::~EventLoop()
{
    
}

void EventLoop::StartLoop()
{
    assert(!Loop_Flag);
    
    while (Loop_Flag)
    {
        this->Events_List = GetReadyEvents();
        for (auto event : Events_List)
        {
            // 一个线程对应一个EventLoop，因此不存在竞态条件
            event->handleEvent();
        }
    }
}