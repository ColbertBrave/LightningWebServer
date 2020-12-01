#include "Timer.h"
#include <sys/time.h>

TimerNode::TimerNode(std::shared_ptr<HttpRequest> request, int timeout): RequestPtr(request)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    this->ExpiredTime = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
    Status = TimerNodeStatus::VALID;
}

TimerNode::~TimerNode() 
{
    RequestPtr->CloseHttp();
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

// 将TimerNode和request解绑，解绑发生在完成httprequest响应后，
// 因此将该定时器节点置为过期，等待被清除
void TimerNode::DetachHttpRequest()
{
    RequestPtr.reset();
    Status = TimerNodeStatus::EXPIRED;
}

TimerQueue::TimerQueue() {}

TimerQueue::~TimerQueue() {}

void TimerQueue::AddTimerNode(TimerNode &node)
{
    // 添加TimerNode时，TimerNode在构造时已经绑定了request(shared_ptr)
    // 现在让request绑定定时器节点(通过weak_ptr绑定)
    std::shared_ptr<TimerNode> NodePtr = std::make_shared<TimerNode>(node);
    NodePtr->RequestPtr->TimerNodeWPtr = NodePtr;
    TimerPriorityQueue.push(NodePtr);
}

void TimerQueue::RemoveExpiredTimerNode()
{
    while (!TimerPriorityQueue.empty())
    {
        std::shared_ptr<TimerNode> NodePtr = TimerPriorityQueue.top();
        if (NodePtr->CheckStatus())
        {
            // 若尚未超时，节点有效
            return;
        }

        TimerPriorityQueue.pop();
    }
}