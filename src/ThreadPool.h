#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <pthread.h>

#include <memory>
#include <vector>

#include "Worker.h"
const int MAX_THREAD_NUM = 16;

class ThreadPool
{
private:
    //std::list<HttpRequest*>          RequestList;        // 思考Request_List和ThreadID_List选择这样的数据结构的原因
    //static const int            MAXThreadNum = 16;       // 作为特例，有序型的const静态数据成员可以在类体中用一常量值初始化
    // 线程Worker的数量是固定的，因此没有使用vector
    // 在使用vector时需要依次make_shared and push_back/embrace_back
    // when using std::array, we can use fill() instead of the complex operation
    // OLD std::vector<std::shared_ptr<Worker>>         WorkerList;    // TODO 其他结构如何: array DONE array更好
    // 补充 由于std::array的第二个模板参数必须是一个常量，在编译时必须是一个确定大小的值，
    // 在下面的代码中，ThreadNum需要在构造函数运行时才能确定大小，因此编译错误。
    // 纠正 暂时修改为vector，用shrink_to_fit控制分配的内存大小
    const unsigned int                                  ThreadNum;
    std::shared_ptr<EventLoop>                          MainLoop;
    std::vector<std::shared_ptr<Worker>>                WorkerList;
    std::vector<std::shared_ptr<EventLoop>>             EventLoopList;
    unsigned int                                        NextLoopIndex;
    
public:
    ThreadPool(std::shared_ptr<EventLoop> loop, int threadNum = 12);
    ~ThreadPool();

    void                        RunThreadPool();
    std::shared_ptr<EventLoop>  GetNextEventLoop();
};
#endif