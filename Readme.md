## TODO
+ 定时器和封装后的互斥锁有无必要，是否可以加入到日志类中，以及是否会影响日志类的性能？
+ 检查各个类是否遵循了RAII

## 项目目的

## 项目结构
### 并发模型
采用One loop per thread


### 服务器
+ 服务器包含一个ThreadPool对象的unique_ptr，一个mainloop对象指针，一个NewRequest指针；
+ mainLoop负责接收客户端发起的连接请求，并且将其封装为NewRequest，然后分发给不同的EventLoop对象；
+ 服务器中会启动线程池，由线程池处理相应的request；

### 线程池
+ 线程池会创建一定数量的线程(Worker类)，每个worker拥有一个eventloop对象指针；
+ 这个eventloop对象不断接收来自mainloop分发的任务，worker运行StartLoop()进行请求的解析和处理；
+ 每个request对象构造时都会包含一个eventloop对象指针，这意味着这个request将被分派给相应的eventloop；

### 定时器

### 日志
- 线程安全单例
- 可拓展日志缓冲区数目
- 要进行测试
- 简单易懂的变量名和函数名
- 如何在不同区域调用同一对象

## 代码规范
+ 在满足需求的基础上，代码的清晰直白比使用各种复杂的特性更加重要；
+ 每个函数都要对传入参数的有效性进行检查；
+ 尽量避免使用C，不得不使用时，使用extern "C"将C语言的代码与C++代码进行分离编译，再统一链接；
+ 类成员均为大驼峰命名，形参和局部变量均为小驼峰
  
## 参考 & 致谢
1. 《后台开发核心技术与应用实践》                   徐晓鑫著
2. 《Linux高性能服务器编程》                       游双著
3. 《Linux多线程服务端编程: 使用muduo C++网络库》   陈硕著
4.  https://github.com/linyacool/WebServer       林亚        
