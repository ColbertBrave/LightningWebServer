#include "Utils.h"

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