#ifndef REDISDATABASE_H
#define REDISDATABASE_H
#include <hiredis/hiredis.h>

class RedisDataBase
{
public:
    RedisDataBase();
    ~RedisDataBase();

private:
    redisContext *redisConnectRet;
}

#endif