#pragma once
#include <mysql/mysql.h>
#include <iostream>

/*
 * author: rose
 * date: 2021.2.19
 * comment: 连接对象类
 */
class MysqlConn {
private:
    MYSQL* conn;
public:
    MysqlConn() {
        conn = new MYSQL();
        mysql_init(conn);

        if (mysql_real_connect(conn, "localhost", "root", "123456", "chat_togethor", 0, NULL, CLIENT_FOUND_ROWS)) {            std::cout << "数据库连接成功!" << std::endl;
        } else {                                            
            std::cout << "数据库连接失败!" << std::endl;                                                           
        }                    
    }

    ~MysqlConn() {
        mysql_close(conn);
    }

    MYSQL* get_conn() {
        return conn;
    }
};
