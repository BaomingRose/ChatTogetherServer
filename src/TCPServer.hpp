#pragma once
#include <jsoncpp/json/json.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>  //负责ip点分十进制和整型之间的转换
#include <netinet/in.h> //负责字节序的转换
#include "Online_Map.hpp"
#include "./dao/ConnTool.hpp"
#include "./dao/UserDao.hpp"
#include "./dao/ChatDao.hpp"
#include "Msg_Pool.hpp"

#define MOST_CONNET_NUM 5
//port : sockaddr_in.sin_port
//ip : sockaddr_in.sin_addr.s_addr //网络字节序

/*
 * author: rose
 * date: 2021.2.20
 * comment: 创建Client_Sock是将描述符和客户端信息传递给线程
 */
struct Client_Sock {
    int cli_fd;
    sockaddr_in cli_addr;
    ConnTool* tool;
    Online_Map* s_ptr;
    Msg_Pool* ms_pool;

    Client_Sock(int fd, sockaddr_in addr, ConnTool* t, Online_Map* s, Msg_Pool* p) {
        cli_fd = fd;
        cli_addr = addr;
        tool = t;
        s_ptr = s;
        ms_pool = p;
    }
};


/*
 * author: rose
 * date: 2021.2.21
 * comment: 该类用于接收并处理用户的登录注册请求
 */
class TCPServer {
private:
    int sock;
private:
    //因为类内函数，默认第一个参数为this，所以要设为static
    static void* handle(void* ptr) {
        struct Client_Sock* cli_ptr = (struct Client_Sock*)ptr;
        int fd = cli_ptr->cli_fd;
        short port = ntohs(cli_ptr->cli_addr.sin_port);
        int ip = ntohl(cli_ptr->cli_addr.sin_addr.s_addr);
        ConnTool* tool_ptr = cli_ptr->tool;
        Online_Map* map_ptr = cli_ptr->s_ptr;
        Msg_Pool* pool_ptr = cli_ptr->ms_pool;

        std::cout << "tcp connect from: " << ip << ":" << port << "访问成功" << std::endl;

#if 0 //debug：连接是否正常
        char buf[1024];
        while (1) {
            bzero(buf, 1024);
            recv(fd, buf, 1024, 0);
            std::cout << "内容为：" << buf << std::endl;
            send(fd, buf, 1024, 0);
        }

#endif
        
#if 1
        //因为是短连接，只需要一次读写，不需要
        char* buf = new char[261];
        bzero(buf, 261);
        recv(fd, buf, 261, 0);

        //debug:
        //std::cout << "tcp recieve data：" << buf << std::endl;

        //前4个字节为请求类型
        int state = *((int*)buf);
        char* data = buf + 4;

        switch(state) {
            case 1: //登录
                handle_log(fd, data, tool_ptr, pool_ptr, map_ptr);
                break;
            case 2: //注册
                handle_sign_up(fd, data, tool_ptr);
                break;
            default:  //其他的东西不做处理，直接断开连接
                break;
        }
#endif

        delete[] buf;
        free(cli_ptr);
        shutdown(fd, SHUT_RDWR);
        std::cout << "tcp connect break: " << ip << ":" << port << "连接断开" << std::endl;
        return NULL;
    }

    static void handle_log(int fd, char* data,  ConnTool* tool, Msg_Pool* pool, Online_Map* on_m) {
       //首先、拿到账号、密码
        std::string s_data(data);
       Json::Reader reader;
       Json::Value val;
       reader.parse(s_data, val, false);
       
       int id = val["userid"].asInt();
       std::string password = val["password"].asString();
       
       //然后去数据库比对密码
       MysqlConn* conn = tool->get_conn();
       UserDao dao(conn);
       std::string user_name;
       bool log_res = dao.check_user(id, password, user_name);
       //登录失败
       if (!log_res) {
           //回复一个，登良失败
           int reply = 2;
           send(fd, &reply, sizeof(reply), 0);
           //return之前记得归还连接对象
           tool->push_conn(conn);
           return;
       }
       tool->push_conn(conn);
       //正确，则拉取记录，和在线表，并返回给客户
       //获取十条记录
       int reply = 1;
       send(fd, &reply, sizeof(reply), 0);
       MysqlConn* conn2 = tool->get_conn();
       ChatDao dao2(conn2);
       std::vector<std::string> v = dao2.get_records();
       tool->push_conn(conn2);
       char records[1024];
       bzero(records, sizeof(records));
       char* begin = records;
       for (const auto& s : v) {
           memcpy(begin, s.c_str(), s.size());
           begin += s.size();
           *begin = '\n';
           begin++;
       }
       send(fd, records, sizeof(records), 0);
       //获取在线用户
       std::vector<std::string> users = on_m->get_users();
       char on_users[1024];
       bzero(records, sizeof(records));
       char* user_begin = on_users;
       for (const auto& s : v) {
           memcpy(user_begin, s.c_str(), s.size());
           user_begin += s.size();
           *user_begin = '\n';
           user_begin++;
       }
       //然后开启心跳检查
       while (1) {
           int sig = 0;
           int ret = recv(fd, &sig, sizeof(sig), 0);
           //如果客户端断开连接则，返回0
           if (ret == 0) {
               //用户下线
               on_m->erase(user_name);
               //构造下线
               string msg;
               Json::Value val;
               Json::FastWriter writer;
               val["type"] = 2;
               val["userid"] = id;
               val["username"] = user_name;
               msg = writer.write(val);
               pool->PushMsgToPool(msg);
               break;
           }
       }
    }

    static void handle_sign_up(int fd, char* data, ConnTool* tool) {
        std::string s_data(data);
        Json::Reader reader;
        Json::Value val;
        reader.parse(s_data, val, false);
        std::string username = val["username"].asString();
        std::string password = val["password"].asString();
        
        MysqlConn* conn = tool->get_conn();
        UserDao dao(conn);
        int ret_id = dao.insert(username, password, 0);
        tool->push_conn(conn);
        
        send(fd, &ret_id, sizeof(ret_id), 0);
    }

public:
    TCPServer() : sock(-1) {  };

    //创建套接字，并监听
    void init(short port) {
        struct sockaddr_in address;
        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        inet_pton(AF_INET, "0.0.0.0", &address.sin_addr); //绑定本机ip，将点分十进制转为int
        address.sin_port = htons(port);

        sock = socket(PF_INET, SOCK_STREAM, 0);
        assert(sock >= 0);
        std::cout << "tcp socket successful!" << std::endl;

        int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
        assert(ret != -1);
        std::cout << "tcp bind port:" << port << " successful!" << std::endl;

        ret = listen(sock, MOST_CONNET_NUM);
        assert(ret != -1);
        std::cout << "tcp start listening................................................." << std::endl;
    }

    void start(ConnTool* tool_ptr, Online_Map* map_ptr, Msg_Pool* p) {
        //开始和监听到的客户端建立连接，并处理
        while (1) {
            struct sockaddr_in client;
            socklen_t client_addr_len = sizeof(client);
            //accept会阻塞
            int connfd = accept(sock, (struct sockaddr*)&client, &client_addr_len);
            if (connfd < 0) {
                std::cout << "tcp accept error: " << errno << std::endl;
            }

            //将栈里空间拷贝到堆中
            Client_Sock* cli_sock_ptr = new Client_Sock(connfd, client, tool_ptr, map_ptr, p);

            //创建线程处理连接
            pthread_t tid;
            int ret = pthread_create(&tid, NULL, handle, (void*)cli_sock_ptr);
            assert(ret != -1);
            if (ret == -1) {
                return ;
            }
        }
    }
};
