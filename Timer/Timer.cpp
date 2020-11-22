#include "Timer.h"
#include <sys/time.h>

TimerNode::TimerNode(std::shared_ptr<HttpRequest> request, int timeout): Request(request)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    this->ExpiredTime = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
    Status = TimerNodeStatus::VALID;
}

TimerNode::~TimerNode() 
{
    Request->CloseHttp();
}

// 更新节点的超时时间
void TimerNode::UpateTimeNode(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    this->ExpiredTime = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

// 获取节点的过期时间
size_t TimerNode::GetExpiredTime()
{
    return this->ExpiredTime;
}

// 检查节点的超时时间是否已经到达
bool TimerNode::CheckStatus()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
    if (temp < ExpiredTime)
        return true;
    else 
    {
        Status = TimerNodeStatus::EXPIRED;
        return false;
    }
}

TimerQueue::TimerQueue() {}

TimerQueue::~TimerQueue() {}

void TimerQueue::AddTimerNode(TimerNode &node)
{
    TimeNodePtr NodePtr = std::make_shared<TimerNode>(node);
    TimerPriorityQueue.push(NodePtr);
}

void TimerQueue::RemoveExpiredTimerNode()
{
    while (!TimerPriorityQueu.empty())
    {
        TimeNodePtr NodePtr = TimerPriorityQueue.top();
        if (NodePtr->CheckStatus())
        {
            // 若尚未超时，节点有效
            return;
        }

        TimerPriorityQueue.pop();
    }
}