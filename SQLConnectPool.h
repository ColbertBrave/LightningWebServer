#ifndef SQLCONNECTPOOL_H
#define SQLCONNECTPOOL_H

#include <mysql/mysql.h>
/*
    单例模式创建、连接池代码实现、RAII机制释放数据库连接。
*/
class SQLConnectPool
{
public:
    SQLConnectPool();
    ~SQLConnectPool();

private:

}

#endif
