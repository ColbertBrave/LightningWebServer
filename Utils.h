#ifndef UTILS_H
#define UTILS_H
#include <sys/epoll.h>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>

bool AddFd(int epoll_fd, int fd);
bool RemoveFd(int epoll_fd, int fd);
bool SetSocketNonBlocking(int fd);
int SetListenFd(int port = 8888);

#endif