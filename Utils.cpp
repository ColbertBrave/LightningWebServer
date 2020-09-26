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