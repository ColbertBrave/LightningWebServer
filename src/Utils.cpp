#include "Utils.h"
#include "string.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
const ssize_t MAX_BUFF = 4096;

bool AddFd(int epoll_fd, int fd)
{
    epoll_event event = EPOLLIN | EPOLLRDHUP | EPOLLET;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event); // event设置要监听的事件类型
    if (!ret)
    {
        LOG << "Fail to add fd into epoll\n";
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
        LOG << "Fail to remove fd from epoll\n";
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
        throw std::exception();
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));       // TODO 用C++特性改写 DONE 没有必要
    server_addr.sin_family = AF_INET;        // IPv4协议族
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);

    // 允许重用本地地址和端口(结束连接后可以立即重启)
    int flag = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    if (bind(listen_fd, (struct scokaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        LOG <<"Failed to bind server address.\n";
        throw std::exception();
    }
    return listen_fd;
}

ssize_t Read(int fd, std::string &buffer)
{
    // 循环读取缓冲区的内容，直到读取完毕
    ssize_t sumReadBytes = 0;
    char tempBuffer[MAX_BUFF];
    while (true)
    {
        ssize_t onceReadBytes = read(fd, tempBuffer, MAX_BUFF);
        if (onceReadBytes == 0)
        {
            return sumReadBytes;
        }

        if (onceReadBytes < 0) && (errno == EINTR)
        {
            continue;
        }

        if (onceReadBytes < 0) && (errno == EAGAIN)
        {
            return sumReadBytes;
        }

        if (onceReadBytes < 0)
        {
            LOG << "Failed to read data from fd\n";
            return -1;
        }

        sumReadBytes += onceReadBytes;
        buffer += std::string(tempBuffer, tempBuffer + onceReadBytes);
    }
}

// TODO 读和写的关系似乎混淆了, 还有缓冲区似乎也有些问题
ssize_t Write(int fd, std::string &buffer)
{
    ssize_t alreadyWritten = 0;
    ssize_t leftBytes = buffer.size();
    const char *writeBuf = buffer.c_str();
    while (true)
    {
        ssize_t onceReadBytes = write(fd, write, leftBytes);
        if (leftBytes == 0)
        {
            return; // TODO
        }

        if ((onceReadBytes < 0) && (errno == EINTR))
        {
            continue;
        }

        if ((onceReadBytes < 0) && (errno == EAGAIN))
        {
            break;
        }

        if (onceReadBytes < 0)
        {
            return -1;
        }

        alreadyWritten += onceReadBytes;
        leftBytes -= onceReadBytes;
        writeBuf += onceReadBytes;
    }
    return alreadyWritten;
}