/* ************************************************************************
> File Name:     abstract_protocol.h
> Author:        Luncles
> 功能:          通用抽象协议基类，后面客户端和服务端通讯的协议都基于此
> Created Time:  2023年08月07日 星期一 18时34分03秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_CODER_ABSTRACT_PROTOCOL_H
#define MYROCKETRPC_NET_CODER_ABSTRACT_PROTOCOL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>

namespace myRocket
{
  // 将这个基类定义成一个含有智能指针的类
  struct AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol>
  {
  public:
    using myAbstractProtocolPtr = std::shared_ptr<AbstractProtocol>;

  private:
  };
}

#endif