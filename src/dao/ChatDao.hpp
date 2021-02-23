#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "MysqlConn.hpp"
#include "ConnTool.hpp"

/*
 * author: rose
 * date:2021.2.20
 * comment: 聊天记录的持久
 */
class ChatDao {
private:
    MYSQL* _conn;
public:
    ChatDao(MysqlConn* conn) {
        if (conn == NULL) {
            std::cout << "ChatDao(): failed！" << std::endl;
        }
        _conn = conn->get_conn();
    }

    //插入新记录
    int insert(const std::string& msg, const std::string& username, const std::string& date) {
        std::string sql = "insert into chat_history(msgtime, msgsrc, msg) values('" + date + "','" + username + "','" + msg + "')";
        std::cout << "insert new chat_history：" << sql << std::endl;
        int res = mysql_query(_conn, sql.c_str());    
        if (res) {    
            std::cout << "insert new chat_history：" << username << " failed!" << std::endl;    
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

    //返回最后十条记录
    std::vector<std::string> get_records() {
        std::vector<std::string> res;
        std::string sql = "select * from (select msgid, msg from chat_history order by msgid desc limit 10) tmp order by msgid";
        mysql_query(_conn, sql.c_str());    

        MYSQL_RES* res_ptr = mysql_store_result(_conn);
        if (!res_ptr) {    
            std::cout << "query all failed" << std::endl;    
            return res;
        }    

        int row = mysql_num_rows(res_ptr);    

        for (int i = 0; i < row; ++i) {    
            MYSQL_ROW result_row = mysql_fetch_row(res_ptr);    
            res.push_back(std::string(result_row[1]));
        }    

        mysql_free_result(res_ptr);
        return res;
    }
};
