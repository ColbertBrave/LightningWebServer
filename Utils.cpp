#include "Utils.h"

bool AddFd(int epoll_fd, int fd)
{

    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, struct epoll_event *event); // todo
}