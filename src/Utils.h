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

// 从fd的缓冲区中读取内容到buffer中
ssize_t ReadFromBuffer(int fd, std::string &buffer);

// 将buffer的内容写至fd的缓冲区中
ssize_t WriteToBuffer(int fd, std::string &buffer);

#endif