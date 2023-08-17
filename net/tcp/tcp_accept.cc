/* ************************************************************************
> File Name:     tcp_accept.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月03日 星期四 21时11分01秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "myRocketRPC/common/log.h"
#include "tcp_accept.h"

namespace myRocketRPC
{
  TcpAccept::TcpAccept(NetAddr::myNetAddrPtr localAddr) : myLocalAddr(localAddr)
  {
    if (!localAddr->CheckAddrValid())
    {
      ERRORLOG("invalid local address [%s]", localAddr->ToString().c_str());
      exit(-1);
    }

    myFamily = myLocalAddr->GetFamily();

    // socket()
    myListenfd = socket(myFamily, SOCK_STREAM, 0);
    if (myListenfd < 0)
    {
      ERRORLOG("invalid listenfd [%d]", myListenfd);
      exit(-1);
    }

    // 将端口设置为可重用
    int reuse = 1;
    if (setsockopt(myListenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)))
    {
      ERRORLOG("setsockopt REUSEADDR error, errno=[%d], error=[%s]", errno, strerror(errno));
    }

    // bind()
    socklen_t socklen = myLocalAddr->GetSockAddrLen();
    if (bind(myListenfd, myLocalAddr->GetSockAddr(), socklen) < 0)
    {
      ERRORLOG("bind error! errno=[%d], error=[ %s]", errno, strerror(errno));
      exit(-1);
    }

    // listen()
    if (listen(myListenfd, 1000) != 0)
    {
      ERRORLOG("listen error! errno=[%d], error=[%s]", errno, strerror(errno));
      exit(-1);
    }

    // 到这里一个监听fd就绑定完毕了
  }

  TcpAccept::~TcpAccept()
  {
  }

  // 接受客户端连接的封装函数
  std::pair<int, NetAddr::myNetAddrPtr> TcpAccept::Accept()
  {
    if (myFamily == AF_INET)
    {
      struct sockaddr_in clientAddr;
      socklen_t clientAddrLen = sizeof(clientAddr);
      memset(&clientAddr, 0, clientAddrLen);

      int clientfd = accept(myListenfd, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrLen);
      if (clientfd < 0)
      {
        ERRORLOG("accept error! errno=[%d], error=[%s]", errno, strerror(errno));
      }

      // 调用了IPNetAddr(struct sockaddr_in& addr);
      IPNetAddr::myNetAddrPtr clientAddrPtr = std::make_shared<IPNetAddr>(clientAddr);
      INFOLOG("A client have accepted success, client address [%s]", clientAddrPtr->ToString().c_str());

      return std::make_pair(clientfd, clientAddrPtr);
    }

    return std::make_pair(-1, nullptr);
  }

  // 获得监听套接字
  int TcpAccept::GetListenFD()
  {
    return myListenfd;
  }
}
