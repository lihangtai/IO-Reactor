#include "InetAddr.h"
#include <string.h>

InetAddr::InetAddr(){
    memset(&addr_, 0, sizeof(addr_));
}

InetAddr::InetAddr(int port, const char* ip){
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_port = htons(port);
    addr_.sin_family = AF_INET;

    inet_pton(AF_INET, ip, &addr_.sin_addr.s_addr);
}

std::string InetAddr::toIp()const{
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddr::toIpPort()const{

    char buf[100] = {0};
    sprintf(buf, "%s:%d", this->toIp().c_str(), this->toPort());
    return buf;
}

int InetAddr::toPort()const{

    return ntohs(addr_.sin_port);
}
