#include "Utils.h"
#include "string.h"

bool AddFd(int epoll_fd, int fd)
{
    epoll_event event = EPOLLIN | EPOLLRDHUP | EPOLLET;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event); // event设置要监听的事件类型
    if (!ret)
    {
        std::cout << "Fail to add fd into epoll" << std::endl;
        return false;
    }
    return true;
}

bool RemoveFd(int epoll_fd, int fd)
{
    int ret = epoll_ctl(epoll_fd, fd, EPOLL_CTL_DEL);
    close(fd);      // 记得关闭文件描述符
    if (!ret)
    {
        std::cout << "Fail to remove fd from epoll" << std::endl;
        return false;
    }
    return true;
}

bool SetSocketNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1) 
    {
        return false;
    }
    
    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1)
    {
        return false;
    }
    return true;
}

int SetListenFd(int port)
{
    if (port > 65535 && port < 1024)
    {
        LOG << "Invalid server port\n";    // TODO 改为写入到日志中 DOING 未测试
        throw std::exception()
    }

    struct sockaddr_in *server_addr;
    bzero(server_addr, sizeof(*server_addr));       // TODO 用C++特性改写
    server_addr->sin_family = AF_INET;        // IPv4协议族
    server_addr->sin_port = htons(port);
    server_addr->sin_addr->s_addr = htonl(INADDR_ANY);
    listen_fd = socket(PF_INET, SOCK_STREAM, 0);

    // 允许重用本地地址和端口(结束连接后可以立即重启)
    int flag = 1;
    int listen_fd;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    if (bind(this->listen_fd, server_addr, typeof(server_addr)) < 0)
    {
        LOG <<"Failed to bind server address.\n";
        throw std::exception();
    }
    return listen_fd;
}