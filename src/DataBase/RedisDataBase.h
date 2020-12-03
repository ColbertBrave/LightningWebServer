#ifndef REDISDATABASE_H
#define REDISDATABASE_H
#include <hiredis/hiredis.h>
#include <string>

/*
    使用Redis存储用户名和密码
    实现了注册，验证，删除用户功能
    清理不活跃用户
*/
class RedisDataBase
{
public:
    RedisDataBase(const char* redis_IP, int redis_port);
    ~RedisDataBase();
    typedef struct UserInfo
    {
        std::string userName;
        std::string userPassword;
    };

    bool RegisterUser(UserInfo user_info);
    bool DeleteUser(UserInfo user_info);
    bool VerifyUser(UserInfo user_info);
    bool CleanInactiveUser();

private:
    redisContext *redisConnectRet;
}

#endif