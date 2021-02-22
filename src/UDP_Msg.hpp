#pragma once
#include <jsoncpp/json/json.h>
#include <iostream>
#include <string.h>
#include <unordered_map>
#include <string>
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Msg_Pool.hpp"
#include "Online_Map.hpp"
#include "./dao/ConnTool.hpp"
#include "./dao/ChatDao.hpp"

#define MSG_PORT_DEFAUT 9999

/*
 * author: rose
 * date: 2020.2.21
 * commnet:UDP收发消息
 */
class UDP_Msg {
private:
    int sock;
    ConnTool* tool;
    Msg_Pool* pool;
    Online_Map* on_map;

private:
    static void* recieve(void* ptr) {
        UDP_Msg* self = (UDP_Msg*)ptr;
        Msg_Pool* pool = self->pool;
        std::cout << "udp socket start recieve thread....................................." << std::endl;

        //循环接收数据
        while (1) {
            char buf[1024];
            sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);
            recvfrom(self->sock, buf, sizeof(buf), 0, (sockaddr*)&addr, &addr_len);
            std::string msg(buf);

            //std::cout << "udp recieve data：" << msg << std::endl;

            try {
                //如果是上线信息，需要修改map
                Json::Reader reader;
                Json::Value val;
                reader.parse(msg, val, false);
                int type = val["type"].asInt();
                if (type == 1) {
                    Online_Map* m_ptr = self->on_map;
                    std::string username = val["username"].asString();
                    m_ptr->insert(username, addr);
                }
            } catch(std::exception e) {
                continue;
            }

            //不管什么都需要放入消息队列
            pool->PushMsgToPool(msg);
            //插入到数据库
            ConnTool* tool = self->tool;
            MysqlConn* conn = tool->get_conn();
            ChatDao dao(conn);
            dao.insert(msg, "null", "null");

            tool->push_conn(conn);
#if 0 //debug
            Online_Map* on_map = self->on_map;
            on_map->insert("rose", addr);
            std::cout << "recv:" << msg << std::endl;
#endif
        }

        return NULL;
    }

    static void* broad_send(void* ptr) {
        UDP_Msg* self = (UDP_Msg*)ptr;
        Msg_Pool* pool = self->pool;
        Online_Map* on_map = self->on_map;

        std::cout << "udp socket start broad_send thread.................................." << std::endl;
        while (1) {
            string msg;
            pool->PopMsgFromPool(msg);
            std::vector<sockaddr_in> v = on_map->get_addrs();
            for (const auto& addr : v) {
                sendto(self->sock, msg.c_str(), msg.size(), 0, (sockaddr*)&addr, sizeof(addr));
                std::cout << "udp broad_send: " << msg << std::endl;
            }
        }

        return NULL;
    }

public:
    UDP_Msg(Msg_Pool* p, Online_Map* m, ConnTool* t) : sock(-1), tool(t), pool(p), on_map(m)  {  }

    ~UDP_Msg() {
        if (sock >= 0) {
            shutdown(sock, SHUT_RDWR);
            sock = 0;
        }
    }

    //给套接字绑定端口
    void init(short port = MSG_PORT_DEFAUT) {
        sockaddr_in address;
        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);
        address.sin_port = htons(port);

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        assert(sock >= 0);
        //std::cout << "udp sock fd is " << sock << std::endl;
        int ret = bind(sock, (sockaddr*)&address, sizeof(address));
        assert(ret != -1);
        std::cout << "udp socket bind " << port << " successful!" << std::endl;
        if (ret == -1) {
            return;
        }
    }

    void start() {
        //需要两个线程，一个收线程，一个群发线程
        pthread_t tid1;
        pthread_t tid2;

        //需要把自己的类传进来，获取几个需要的Date Structure
        pthread_create(&tid1, NULL, recieve, this);
        pthread_create(&tid2, NULL, broad_send, this);

    }
};
