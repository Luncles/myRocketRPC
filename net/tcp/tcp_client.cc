/* ************************************************************************
> File Name:     tcp_client.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月07日 星期一 18时19分26秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tcp_client.h"
#include "../../common/log.h"
#include "../fd_event_group.h"

namespace myRocket
{

  TcpClient::TcpClient(NetAddr::myNetAddrPtr serverAddress) : myServerAddr(serverAddress)
  {
    // 因为客户端线程没有eventloop，所以会直接创建一个，注意创建后要运行loop函数才会进入实际运行
    myEventLoop = EventLoop::GetCurrentEventLoop();

    // 直接创建一个socket fd
    myFD = socket(serverAddress->GetFamily(), SOCK_STREAM, 0);

    if (myFD < 0)
    {
      ERRORLOG("TcpClient::TcpClient() error! fail to create fd");
      return;
    }

    // 直接在本地创建fdevent池，从fdevent池里取一个事件出来
    myFDEvent = FDEventGroup::GetFDEventGroup()->GetFDEvent(myFD);
    // 其实在创建tcp connection的时候也会将fd设置为非阻塞，但是这里为了保险起见，还是先设置一下
    myFDEvent->SetNonBlock(myFD);

    // 创建tcp connection
    myTcpConnection = std::make_shared<TcpConnection>(myEventLoop, myFD, 128, nullptr, myServerAddr, TcpConnectionByClient);
    myTcpConnection->SetConnectionType(TcpConnectionByClient);
  }

  TcpClient::~TcpClient()
  {
    DEBUGLOG("~TcpClient");

    // 要关闭fd
    if (myFD > 0)
    {
      close(myFD);
    }

    // 清理一些指针
    if (!myEventLoop)
    {
      delete myEventLoop;
      myEventLoop = nullptr;
    }

    if (!myFDEvent)
    {
      delete myFDEvent;
      myFDEvent = nullptr;
    }
  }

  // 异步地进行connect
  // 借助epoll，如果connect完成了，会触发回调函数
  void TcpClient::ConnectServer(std::function<void()> ConnectCallBack)
  {
    int ret = ::connect(myFD, myServerAddr->GetSockAddr(), myServerAddr->GetSockAddrLen());

    // 连接成功，在非阻塞的socket中调用connect，连接没有立即建立时，会返回-1，如果errno=EINPROGRESS，可以调用epoll监听这个连接失败的socket上的可写事件
    // epoll返回后，再利用getsockopt来读取错误码并清除该socket上的错误。如果错误码是0，表示连接成功建立，否则连接失败。
    if (ret == 0)
    {
      DEBUGLOG("connect [%s] succes, ret=[%d]", myServerAddr->ToString().c_str(), ret);
      myTcpConnection->SetState(Connected);
      if (ConnectCallBack)
      {
        ConnectCallBack();
      }
      // 还要判断eventloop是不是打开了
      // if (!myEventLoop->isLooping())
      // {
      //   myEventLoop->Loop();
      // }
    }
    else if (ret == -1)
    {
      if (errno == EINPROGRESS)
      {
        // 监听可写事件
        myFDEvent->Listen(FDEvent::OUT_EVENT, [this, ConnectCallBack, ret]()
                          {
          int error = 0;
          socklen_t errorLen = sizeof(error);
          if (getsockopt(myFD, SOL_SOCKET, SO_ERROR, &error, &errorLen) < 0) {
            ERRORLOG("get socket option failed, fd=[%d], server address=[%s]", myFD, myServerAddr->ToString().c_str());
            return;
          }
          bool isConnectSuccess = false;
          if (error == 0) {
            DEBUGLOG("connect [%s] success, ret=[%d]", myServerAddr->ToString().c_str(), ret);
            myTcpConnection->SetState(Connected);
            isConnectSuccess = true;
            
          }
          else {
            ERRORLOG("connect error! errno=[%d], error=[%s]", errno, strerror(errno));
          }
          // 可写事件触发后要去掉可写事件的监听，不然会一直触发
          myFDEvent->CancelEvent(FDEvent::OUT_EVENT);
          myEventLoop->AddEpollEvent(myFDEvent); 

          // 连接成功了才执行回调函数
          if (isConnectSuccess && ConnectCallBack) {
              ConnectCallBack();
          } });

        myEventLoop->AddEpollEvent(myFDEvent);
        // 还要判断eventloop是不是打开了
        if (!myEventLoop->isLooping())
        {
          myEventLoop->Loop();
        }
      }
      else
      {
        ERRORLOG("connect error! errno=[%d], error=[%s]", errno, strerror(errno));
      }
    }
  }

  // 异步地发送数据
  // 借助epoll，如果发送成功，会触发回调函数 void WriteCallBack(AbstractProtocol::myAbstractProtocolPtr message)
  void TcpClient::WriteMessage(AbstractProtocol::myAbstractProtocolPtr message, std::function<void(AbstractProtocol::myAbstractProtocolPtr)> WriteCallBack)
  {
    // 1、将message对象和回调函数写入到connection的sendbuffer中
    // 2、启动监听可写事件
    myTcpConnection->PushSendMessage(message, WriteCallBack);
    DEBUGLOG("message id=[%s]", message->myMessageID.c_str());
    myTcpConnection->ListenWrite();
  }

  // 异步地接收数据
  // 借助epoll，如果接收成功，会触发回调函数 void ReadCallBack(AbstractProtocol::myAbstractProtocolPtr message)
  void TcpClient::ReadMessage(const std::string &message, std::function<void(AbstractProtocol::myAbstractProtocolPtr)> ReadCallBack)
  {
    // 1、将message对象和回调函数写入到connection的recvbuffer中
    // 2、启动监听可读事件
    myTcpConnection->PushRecvMessage(message, ReadCallBack);
    myTcpConnection->ListenRead();
  }

  // 关闭客户端的eventloop
  void TcpClient::Stop()
  {
  }

  // 获取本地地址
  IPNetAddr::myNetAddrPtr TcpClient::GetLocalAddress()
  {
    return myLocalAddr;
  }

  // 获取服务器地址
  IPNetAddr::myNetAddrPtr TcpClient::GetServerAddress()
  {
    return myServerAddr;
  }
}