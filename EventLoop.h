#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include <vector>

#include <pthread.h>

#include "MutexLock.h"


/*
    每个EventLoop至少一个线程，每个线程至多处理一个EventLoop，每个EventLoop可以被其他线程访问，因此要保证可重入

*/
class EventLoop
{
private:
    std::vector<Event> Events_List;      // TODO Event or HttpRequest
    std::vector<Event> GetReadyEvents();
    bool Loop_Flag;
    pthread_t Thread_ID;

public:
    EventLoop();
    ~EventLoop();

    void StartLoop();

};

#endif