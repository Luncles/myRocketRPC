/* ************************************************************************
> File Name:     net_addr.h
> Author:        Luncles
> 功能:          对网络地址相关操作进行封装
> Created Time:  2023年08月03日 星期四 20时18分03秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_TCP_NET_ADDR_H
#define MYROCKETRPC_NET_TCP_NET_ADDR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>

namespace myRocket
{
  // 定义一个基类，可以让其他协议重用
  class NetAddr
  {
  public:
    using myNetAddrPtr = std::shared_ptr<NetAddr>;

    virtual sockaddr *GetSockAddr() = 0;

    virtual socklen_t GetSockAddrLen() = 0;

    // 当前协议属于哪个协议族
    virtual int GetFamily() = 0;

    // 转成字符串
    virtual std::string ToString() = 0;

    // 校验是否是有效地址
    virtual bool CheckAddrValid() = 0;

    virtual ~NetAddr() {}

  private:
  };

  // TCP地址类 IP：端口
  class IPNetAddr : public NetAddr
  {
  public:
    static bool StaticCheckAddrValid(std::string &addr);

  public:
    // 支持三种构造方式：1、ip，port；2，ip:port；3、sockaddr_in
    IPNetAddr(const std::string &ip, uint16_t port);
    IPNetAddr(const std::string &addr);
    IPNetAddr(struct sockaddr_in &addr);

    ~IPNetAddr();

    sockaddr *GetSockAddr();

    socklen_t GetSockAddrLen();

    int GetFamily();

    std::string ToString();

    bool CheckAddrValid();

  private:
    std::string myIP;
    uint16_t myPort{0};

    struct sockaddr_in myAddr;
  };
}

#endif