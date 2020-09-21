#include "RedisDataBase.h"
#include <iostream>


RedisDataBase::RedisDataBase(const char *redis_IP = "127.0.0.1", int redis_port = 6379)
{
    this->redisConnectRet = redisConnect(const char *redis_IP, int redis_port);
    if (redisConnectRet == NULL || redisConnectRet->err)  // redisContext is not thread-safe
    {
        if (redisConnectRet) 
        {
            std::cout << "Error: " << redisConnectRet->errstr << std::endl;
            throw std::exception();
        }
        else 
        {
            std::cout << "Can't allocate redis context" << std::endl;
            throw std::exception();
        }
    }
}

RedisDataBase::~RedisDataBase)()
{
    void redisFree(this->redisConnectRet);
}