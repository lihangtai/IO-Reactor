#pragma once

#include "Channel.h"
#include "Epoll.h"
#include "Socket.h"
#include "util/util.h"
#include "EventLoop.h"
#include "InetAddr.h"
#include <memory>

// new module
#include "Acceptor.h"
#include "Buffer.h"
#include "Connection.h"
#include <map>


class Server{

    public:
        using connectionMap = std::map<int, Connectionptr>

}