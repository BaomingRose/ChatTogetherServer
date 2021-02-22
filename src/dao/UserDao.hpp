#pragma once
#include <mysql/mysql.h>
#include <string>
#include <string.h>
#include "MysqlConn.hpp"

/*
 * author: rose
 * date: 2021.2.19
 * commnet: 用户表的持久操作
 */
class UserDao {
private:
    MYSQL* _conn;
public:
    //构造
    UserDao(MysqlConn* conn) {
        if (conn == NULL) {
            std::cout << "UserDao: 连接对象为空！" << std::endl;
        }
        _conn = conn->get_conn();
    }

    //插入新用户，并且返回插入的id
    int insert(const std::string& user_name, const std::string& password,  int state) {
        std::string s_state = std::to_string(state);
        std::string sql = "insert into user(username, password, state) values('" + user_name + "', '" + password + "', '" + s_state  + "')";
        std::cout << "insert new user：" <<  sql << std::endl;
        int res = mysql_query(_conn, sql.c_str());    
        if (res) {    
            std::cout << "insert " << user_name << " failed!" << std::endl;    
            return -1;    
        }    

        std::string new_sql = "select last_insert_id()";
        res = mysql_query(_conn, new_sql.c_str());
        if (res) {
            std::cout << "last_insert_id() failed!" << std::endl;
            return -1;
        }

        MYSQL_RES* res_ptr = mysql_store_result(_conn);
        MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
        char* last = result_row[0];
        res = atoi(last);

        return res;
    }

    bool check_user(int userid, const std::string& password, std::string& username) {
        std::string s_id = std::to_string(userid);
        std::string sql = "select password,username from user where userid=" + s_id;

        int res = mysql_query(_conn, sql.c_str());    
        if (res) {
            std::cout << "query user failed!" << std::endl;
            return false;
        }

        MYSQL_RES* res_ptr = mysql_store_result(_conn);
        int row = mysql_num_rows(res_ptr);
        if (row <= 0) {
            std::cout << "user is not exist" << std::endl;
            return false;
        }
        MYSQL_ROW result_row = mysql_fetch_row(res_ptr);
        char* pass_word = result_row[0];
        char* user_name = result_row[1];
        username = user_name;
        if (strcmp(pass_word, password.c_str()) == 0) {
            std::cout << "user password error" << std::endl;
            return true;
        }
        
        return false;
    }

    //查询用户状态
    int query_state() {

        return 0;
    }

    //修改用户状态
    bool change_state(int userid, int state) {
        std::string user_id = std::to_string(userid);
        std::string s_state = std::to_string(state);
        std::string sql = "update user set state=" + s_state + " where userid=" + user_id;

        int res = mysql_query(_conn, sql.c_str());
        if (res) {
            std::cout << "change_" << userid <<  " state() failed!" << std::endl;
            return false;
        }
        return true;
    }
};
