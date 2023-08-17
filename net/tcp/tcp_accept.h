/* ************************************************************************
> File Name:     tcp_accept.h
> Author:        Luncles
> 功能:          TCP服务器处理连接的一系列逻辑的封装，在这里完成tcp的连接
> Created Time:  2023年08月03日 星期四 21时10分29秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_TCP_TCP_ACCEPT_H
#define MYROCKETRPC_NET_TCP_TCP_ACCEPT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include "net_addr.h"

namespace myRocketRPC
{
  class TcpAccept
  {
  public:
    using myTcpAcceptPtr = std::shared_ptr<TcpAccept>;

    TcpAccept(NetAddr::myNetAddrPtr localAddr);

    ~TcpAccept();

    // 接受客户端连接的封装函数
    std::pair<int, NetAddr::myNetAddrPtr> Accept();

    // 获得监听套接字
    int GetListenFD();

  private:
    NetAddr::myNetAddrPtr myLocalAddr; // 服务器监听地址 ip:port，用基类指针是为了可以实现多态

    int myFamily{-1}; // 协议族

    int myListenfd{-1}; // 监听套接字
  };
}

#endif
