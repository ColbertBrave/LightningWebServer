#ifndef REDISDATABASE_H
#define REDISDATABASE_H
#include <hiredis/hiredis.h>

/*
    使用Redis存储用户名和密码
    实现了注册，验证，删除用户功能
    清理不活跃用户
*/
class RedisDataBase
{
public:
    RedisDataBase();
    ~RedisDataBase();
    typedef struct UserInfo
    {
        string userName;
        string userPassword;
    };

    bool RegisterUser(UserInfo user_info);
    bool DeleteUser(UserInfo user_info);
    bool VerifyUser(UserInfo user_info);
    bool CleanInactiveUser();

private:
    redisContext *redisConnectRet;
}

#endif