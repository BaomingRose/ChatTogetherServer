#include <iostream>
#include "./dao/ConnTool.hpp"
#include "Msg_Pool.hpp"
#include "Online_Map.hpp"
#include "TCPServer.hpp"
#include "UDP_Msg.hpp"

int main() {
    //创建5个连接对象
    ConnTool* tool = new ConnTool(5);
    Online_Map* on_map = new Online_Map();
    Msg_Pool* pool = new Msg_Pool();
    
    UDP_Msg udp_svr(pool, on_map, tool);
    udp_svr.init();
    udp_svr.start();

    TCPServer tcp_svr;
    tcp_svr.init(9001);
    tcp_svr.start(tool, on_map, pool);

    while (1) {}
    delete tool;
    delete on_map;
    delete pool;
    return 0;
}
