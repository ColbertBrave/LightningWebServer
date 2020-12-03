#ifndef TIMER_H
#define TIMER_H
#include <memory>
#include <queue>

class HttpRequest;
class TimerNode
{
private:
    enum class TimerNodeStatus
    {
        VALID, EXPIRED
    };
    // 每个定时器都绑定了一个HttpRequest指针，用于清理不活跃连接等
    
    size_t ExpiredTime;

public:
    TimerNode(std::shared_ptr<HttpRequest> request, int timeout);
    ~TimerNode();

    void                UpateTimeNode(int timeout);
    size_t              GetExpiredTime();
    bool                CheckStatus();
    TimerNodeStatus     Status;
    void                DetachHttpRequest();
    std::shared_ptr<HttpRequest> RequestPtr;
};

// 仿照std::greater定义了一个函数对象，用于TimerQueue中priority_queue不同TimerNode的比较
struct GreaterTimer
{
    bool operator()(std::shared_ptr<TimerNode> timer_node1, std::shared_ptr<TimerNode> timer_node2) const
    {
        if (timer_node1->GetExpiredTime() > timer_node2->GetExpiredTime())
            return timer_node1->GetExpiredTime();
        else
            return timer_node2->GetExpiredTime();
    }
};


class TimerQueue
{
private:
    typedef std::shared_ptr<TimerNode> TimerNodePtr;
    // 升序队列，小根堆
    std::priority_queue<TimerNodePtr, std::deque<TimerNodePtr>, GreaterTimer> TimerPriorityQueue;

public:
    TimerQueue();
    ~TimerQueue();

    void AddTimerNode(TimerNode& node);
    void RemoveExpiredTimerNode();
};
#endif