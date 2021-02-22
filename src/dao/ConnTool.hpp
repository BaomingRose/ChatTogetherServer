#pragma once
#include <mysql/mysql.h>
#include <queue>
#include <pthread.h>
#include "MysqlConn.hpp"

/*
 * autor: rose
 * date: 2021.2.19
 * conment: 数据库连接池类
 */
class ConnTool {
private:
    size_t _num;
    std::queue<MysqlConn*> conn_tool;
    pthread_mutex_t mtx;
    pthread_cond_t empt;
public:
    //初始化num个连接对象
    ConnTool(int num) : _num(num) {
        pthread_mutex_init(&mtx, NULL);
        pthread_cond_init(&empt, NULL);
        for (int i = 0; i < num; ++i) {
            MysqlConn* conn = new MysqlConn();
            conn_tool.push(conn);
        }
    }

    ~ConnTool() {
        for (size_t i = 0; i < _num; ++i) {
            MysqlConn* cn = get_conn();
            delete cn;
        }
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&empt);
    }

    //从池中获取一个，相当于pop
    MysqlConn* get_conn() {
        pthread_mutex_lock(&mtx);
        //如果为空则等待挂起
        while (0 == conn_tool.size()) {
            pthread_cond_wait(&empt, &mtx);
        }

        MysqlConn* res = conn_tool.front();
        conn_tool.pop();
        pthread_mutex_unlock(&mtx);

        return res;
    }

    //向里push连接对象，数量固定不会满
    void push_conn(MysqlConn* conn) {
        pthread_mutex_lock(&mtx);
        conn_tool.push(conn);
        pthread_mutex_unlock(&mtx);
        //push结束，需要唤醒一下
        pthread_cond_signal(&empt);
    }
};
