#include "RedisDataBase.h"
#include <iostream>
#include <cstring>
#include <string>


RedisDataBase::RedisDataBase(const char *redis_IP = "127.0.0.1", int redis_port = 6379)
{
    this->redisConnectRet = redisConnectWithTimeout(const char *redis_IP, int redis_port);
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

bool RedisDataBase::RegisterUser(const UserInfo &user_info)
{
    string registerCommand;
    registerCommand = "SET " + user_info.userName + " " + user_info.userPassword;
    void *ret = redisCommand(this->redisConnectRet, registerCommand.c_str());
    
    if (!ret)   // 注册失败，返回NULL
    {
        std::cout << "Fail to register the user" << std::endl;
        return false;
    }

    std::cout << "Successfully register the user" << std::endl;
    return true;
}

bool RedisDataBase::VerifyUser(UserInfo user_info)
{
    string verifyCommand;
    verifyCommand = "GET " + user_info.userName + " " + user_info.userPassword;
    void *reply = redisCommand(this->redisConnectRet, verifyCommand.c_str());
    if (reply->type == REDIS_REPLY_NIL) // 若用户名不存在
    {
        std::cout << "The user does not exist" << std::endl;
        return false;
    }
        
    // 用户名存在
    return true;
}

bool RedisDataBase::DeleteUser(UserInfo user_info)
{
    string deleteCommand;
    deleteCommand = "DEL " + user_info.userName + " " + user_info.userPassword;
    void *reply = redisCommand(this->redisConnectRet, deleteCommand.c_str());
    // 如果用户名不存在则忽略
       
    if (reply->integer > 0) // 用户名存在，返回删除的键的长度
    {
        std::cout << "The user has been deleted" << std::endl;
        return true;
    }
    else
    {
        throw std::exception();
    }
}

bool RedisDataBase::CleanInactiveUser()
{
    
}
