#include <iostream>
#include "MysqlConn.hpp"
#include "ConnTool.hpp"
#include "UserDao.hpp"
#include "ChatDao.hpp"
using namespace std;

//测试连接对象类
void testConn() {
    MysqlConn conn;
}

void testTool() {
    ConnTool tool(5);

    for (int i = 0; i < 10; ++i) {
        MysqlConn* conn =  tool.get_conn();
        if (i < 4) {
            tool.push_conn(conn);
        }
        cout << "得到对象" << endl;
    }
}

void testUserDao() {
    ConnTool tool(5);
    MysqlConn* conn = tool.get_conn();
    UserDao dao(conn);
#if 0 //这是一个完整的插入操作，因为如果不重新push会连接对象，析构连接池的时候阻塞
    int res = dao.insert("rose", "1", 0);
    cout << "last_insert:" << res << endl;
    tool.push_conn(conn);
#endif

#if 0 //验证用户登录密码是否正确
    bool res = dao.check_user(4, "3");
    if (res) {
        cout << "登录成功" << endl;
    }
    tool.push_conn(conn);
#endif
    bool res =  dao.change_state(3, 1);
    if (res) {
        cout << "修改成功" << endl;
    } else {
        cout << "修改失败" << endl;
    }
    tool.push_conn(conn);
}

void testChatDao() {
    ConnTool tool(5);
    MysqlConn* conn = tool.get_conn();
    ChatDao dao(conn);
    int res = dao.insert("hello", "rose", "2021.2.20");
    cout << res << endl;

    tool.push_conn(conn);
}

int main() {
    //testTool();
    //testUserDao();
    testChatDao();

    return 0;
}
