#include <iostream>
using namespace std;
#include <jsoncpp/json/json.h>
#include <string>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void test() {
    short port = 9999;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    std::cout << "udp sock fd is " << sock << std::endl;

    //构造目的地址
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    address.sin_port = htons(port);

    //构造发送串
    Json::FastWriter writer;
    Json::Value val;
    string s;
    val["type"] = 1;
    val["userid"] = 1;
    val["username"] = "rose";
    s = writer.write(val);

    cout << "json:" << s << endl;
    int ret = sendto(sock, s.c_str(), s.size(), 0, (sockaddr*)&address, sizeof(address));
    cout << ret << endl;
}

int main() {
    test();

    return 0;
}
