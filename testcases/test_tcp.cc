/* ************************************************************************
> File Name:     test_tcp.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月03日 星期四 22时28分47秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include "myRocketRPC/common/log.h"
#include "myRocketRPC/net/tcp/net_addr.h"
#include "myRocketRPC/common/config.h"
#include "myRocketRPC/net/tcp/tcp_server.h"

void test_tcp_server()
{
  myRocketRPC::IPNetAddr::myNetAddrPtr addrPtr = std::make_shared<myRocketRPC::IPNetAddr>("127.0.0.1", 12355);
  DEBUGLOG("create address [%s]", addrPtr->ToString().c_str());
  myRocketRPC::TCPServer tcpServer(addrPtr);
  tcpServer.Start();
}

int main()
{
  myRocketRPC::Config::SetGlobalConfig("/home/luncles/myRocketRPC/conf/myRocket.xml");
  myRocketRPC::Logger::InitGlobalLogger(1);

  // myRocket::IPNetAddr addr("127.0.0.1", 12366);
  // DEBUGLOG("create address[%s]", addr.ToString().c_str());
  test_tcp_server();
}
