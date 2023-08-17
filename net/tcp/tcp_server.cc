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
#include "myRocketRPC/common/log.h"
#include "tcp_server.h"
#include "tcp_connection.h"
#include "myRocketRPC/common/config.h"

namespace myRocketRPC
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

    myIOThreadGroup = new IOThreadGroup(Config::GetGlobalConfig()->myIOThread);

    myListenFDEvent = new FDEvent(myAcceptor->GetListenFD());

    // 将监听fd加入到epoll监听事件表里
    myListenFDEvent->Listen(FDEvent::IN_EVENT, std::bind(&TCPServer::OnAccept, this));

    // 真正开始epoll监听
    myMainEventLoop->AddEpollEvent(myListenFDEvent);

    // 定时任务：每隔一段时间删除已经断开的连接
    myTimerEvent = std::make_shared<TimerEvent>(myEraseConnectionInterval, true, std::bind(&TCPServer::ClearClosedClientTimer, this));
    myMainEventLoop->AddTimerEvent(myTimerEvent);
  }

  // 当有新客户端连接后需要执行，将客户端地址传给一个io线程进行业务处理
  void TCPServer::OnAccept()
  {
    auto acceptResult = myAcceptor->Accept();
    int clientfd = acceptResult.first;
    NetAddr::myNetAddrPtr clientAddress = acceptResult.second;

    // 更新连接客户端数
    myClientCount++;

    // 把clientfd加到任意的io线程里执行业务逻辑
    // 先分配一个io线程
    IOThread *ioThread = myIOThreadGroup->GetIOThread();
    TcpConnection::myTcpConnectionPtr connection = std::make_shared<TcpConnection>(ioThread->GetEventLoop(), clientfd, 128, myLocalAddress, clientAddress);
    connection->SetState(Connected);
    myClientConnection.insert(connection);
    INFOLOG("TCP server success get client, client fd[%d]", clientfd);
  }

  // 清除已经close的客户端连接
  void TCPServer::ClearClosedClientTimer()
  {
    DEBUGLOG("now clear closed client");
    // 因为只有主线程操作这个连接池，所以不用加锁
    auto it = myClientConnection.begin();
    for (; it != myClientConnection.end();)
    {
      if ((*it) != nullptr && (*it).use_count() > 0 && (*it)->GetState() == Closed)
      {
        DEBUGLOG("TcpConnection [fd:%d] will delete, state=[%d]", (*it)->GetFD(), (*it)->GetState());
        // 指向被删除的连接后面的连接
        it = myClientConnection.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
}