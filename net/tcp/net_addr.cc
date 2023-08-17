/* ************************************************************************
> File Name:     net_addr.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月03日 星期四 20时17分59秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "myRocketRPC/common/log.h"
#include "net_addr.h"

namespace myRocketRPC
{
  bool IPNetAddr::StaticCheckAddrValid(const std::string &addr)
  {
    size_t pos = addr.find_first_of(":");
    if (pos == addr.npos)
    {
      return false;
    }
    std::string ip = addr.substr(0, pos);
    std::string port = addr.substr(pos + 1, addr.length() - pos - 1);

    if (ip.empty() || port.empty())
    {
      return false;
    }

    int iport = std::atoi(port.c_str());
    if (iport <= 0 || iport > 65536)
    {
      return false;
    }

    if (inet_addr(ip.c_str()) == INADDR_NONE)
    {
      return false;
    }

    return true;
  }

  // 支持三种构造方式：1、ip，port；2，ip:port；3、sockaddr_in
  IPNetAddr::IPNetAddr(const std::string &ip, uint16_t port) : myIP(ip), myPort(port)
  {
    memset(&myAddr, 0, sizeof(myAddr));

    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = inet_addr(myIP.c_str());
    myAddr.sin_port = htons(myPort);
  }
  IPNetAddr::IPNetAddr(const std::string &addr)
  {
    // 找到分隔符
    size_t pos = addr.find_first_of(":");
    if (pos == addr.npos)
    {
      ERRORLOG("invalid ipv4 address [%s]", addr.c_str());
      return;
    }
    myIP = addr.substr(0, pos);
    myPort = atoi(addr.substr(pos + 1, addr.length() - pos - 1).c_str());

    memset(&myAddr, 0, sizeof(myAddr));

    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = inet_addr(myIP.c_str());
    myAddr.sin_port = htons(myPort);
  }
  IPNetAddr::IPNetAddr(struct sockaddr_in &addr) : myAddr(addr)
  {
    myIP = std::string(inet_ntoa(myAddr.sin_addr));
    myPort = ntohs(myAddr.sin_port);
  }

  IPNetAddr::~IPNetAddr()
  {
  }

  sockaddr *IPNetAddr::GetSockAddr()
  {
    return reinterpret_cast<struct sockaddr *>(&myAddr);
  }

  socklen_t IPNetAddr::GetSockAddrLen()
  {
    return sizeof(myAddr);
  }

  int IPNetAddr::GetFamily()
  {
    return myAddr.sin_family;
  }

  std::string IPNetAddr::ToString()
  {
    std::string tmp;
    tmp = myIP + ":" + std::to_string(myPort);
    return tmp;
  }

  bool IPNetAddr::CheckAddrValid()
  {
    if (myIP.empty())
    {
      return false;
    }

    if (myPort < 0 || myPort > 65536)
    {
      return false;
    }

    // 检查是不是有效的标准点分ip地址
    if (inet_addr(myIP.c_str()) == INADDR_NONE)
    {
      return false;
    }

    return true;
  }
}