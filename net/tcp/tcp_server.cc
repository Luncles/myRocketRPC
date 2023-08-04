/* ************************************************************************
> File Name:     tcp_server.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月04日 星期五 16时57分59秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include "/home/luncles/myRocketRPC/common/log.h"
#include "tcp_server.h"

namespace myRocket
{
  TCPServer::TCPServer(NetAddr::myNetAddrPtr localAddress) : myLocalAddress(localAddress)
  {
    InitServer();

    INFOLOG("myRocket TcpServer listen success on [%s]", myLocalAddress->ToString().c_str());
  }

  TCPServer::~TCPServer()
  {
    if (myMainEventLoop)
    {
      delete myMainEventLoop;
      myMainEventLoop = nullptr;
    }

    if (myIOThreadGroup)
    {
      delete myIOThreadGroup;
      myIOThreadGroup = nullptr;
    }

    if (myListenFDEvent)
    {
      delete myListenFDEvent;
      myListenFDEvent = nullptr;
    }
  }

  // 启动服务器
  void TCPServer::Start()
  {
    // 先开启io线程，防止主线程开启后有客户端连接找不到io线程处理
    myIOThreadGroup->Start();
    myMainEventLoop->Loop();
  }

  void TCPServer::InitServer()
  {
    myAcceptor = std::make_shared<TcpAccept>(myLocalAddress);

    myMainEventLoop = EventLoop::GetCurrentEventLoop();

    myIOThreadGroup = new IOThreadGroup(2);

    myListenFDEvent = new FDEvent(myAcceptor->GetListenFD());

    // 将监听fd加入到epoll监听事件表里
    myListenFDEvent->Listen(FDEvent::IN_EVENT, std::bind(&TCPServer::OnAccept, this));

    // 真正开始epoll监听
    myMainEventLoop->AddEpollEvent(myListenFDEvent);
  }

  // 当有新客户端连接后需要执行，将客户端地址传给一个io线程进行业务处理
  void TCPServer::OnAccept()
  {
    auto acceptResult = myAcceptor->Accept();
    int clientfd = acceptResult.first;
    NetAddr::myNetAddrPtr clientAddress = acceptResult.second;

    // 更新连接客户端数
    myClientCount++;

    INFOLOG("TCP server success get client, client fd[%d]", clientfd);
  }

  // 清除已经close的客户端连接
  void TCPServer::ClearClosedClientTimer()
  {
  }
}