#include "TCPServer.hpp"
#include "UDP_Msg.hpp"

void testTCPconn() {
    TCPServer tcp;
    tcp.init(9999);
}

void UDPconn() {
    Msg_Pool* pool = new Msg_Pool();
    Online_Map* mp = new Online_Map();
    UDP_Msg udp(pool, mp);
    udp.init(9999);
    udp.start();

    while(1) {}
}

int main() {
    //testTCPconn();
    UDPconn();
    return 0;
}
