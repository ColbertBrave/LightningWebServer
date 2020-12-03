#ifndef UTILS_H
#define UTILS_H
#include <sys/epoll.h>
#include <unistd.h>
#include <netinet/in.h>

#include <iostream>

#include "Logging/Logging.h"

bool AddFd(int epoll_fd, int fd);
bool SetSocketNonBlocking(int fd);
int SetListenFd(int port = 8888);

// 从fd的缓冲区中读取内容到buffer中
ssize_t Read(int fd, std::string &buffer);

// 将buffer的内容写至fd的缓冲区中
ssize_t Write(int fd, std::string &buffer);

#endif