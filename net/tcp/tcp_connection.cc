/* ************************************************************************
> File Name:     tcp_connection.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月05日 星期六 19时39分51秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tcp_connection.h"
#include "../../common/log.h"
#include "../fd_event_group.h"

namespace myRocket
{
  TcpConnection::TcpConnection(EventLoop *eventloop, int fd, int bufferSize, IPNetAddr::myNetAddrPtr serverAddr, IPNetAddr::myNetAddrPtr clientAddr, TcpConnectionType type) : myEventLoop(eventloop), myFD(fd), myServerAddr(serverAddr), myClientAddr(clientAddr), myState(NotConnected), myConnectionType(type)
  {
    myRecvBuffer = std::make_shared<TCPBuffer>(bufferSize);
    mySendBuffer = std::make_shared<TCPBuffer>(bufferSize);

    myFDEvent = FDEventGroup::GetFDEventGroup()->GetFDEvent(myFD);

    myFDEvent->SetNonBlock(myFD);

    myFDEvent->Listen(FDEvent::IN_EVENT, std::bind(&TcpConnection::OnRead, this));
    myEventLoop->AddEpollEvent(myFDEvent);
  }

  TcpConnection::~TcpConnection()
  {
    DEBUGLOG("~TcpConnection");
    if (myEventLoop != nullptr)
    {
      delete myEventLoop;
      myEventLoop = nullptr;
    }
    if (myFDEvent != nullptr)
    {
      delete myFDEvent;
      myFDEvent = nullptr;
    }
  }

  // 读接收缓冲区回调函数
  void TcpConnection::OnRead()
  {
    // 调用系统的read函数，从socket缓冲区读取数据到recvbuffer里面
    if (myState != Connected)
    {
      ERRORLOG("OnRead error! client has already disconnected, addr=[%s]", myClientAddr->ToString().c_str());
      return;
    }

    // 读完标志
    bool isReadAll = false;
    // 关闭标志
    bool isClosed = false;

    while (!isReadAll)
    {
      // 先检查还没有读取数据的空间
      if (myRecvBuffer->WriteRemain() == 0)
      {
        // 没空间就进行扩容
        myRecvBuffer->ResizeBuffer(2 * myRecvBuffer->myBuffer.size());
      }

      int readMax = myRecvBuffer->WriteRemain();

      int writeIndex = myRecvBuffer->GetWriteIndex();

      int ret = read(myFD, &(myRecvBuffer->myBuffer[writeIndex]), readMax);

      // 读取数据成功
      if (ret > 0)
      {
        DEBUGLOG("success read [%d] bytes from addr[%s], client fd=[%d]", ret, myClientAddr->ToString().c_str(), myFD);
        // 要先移动写指针
        myRecvBuffer->MoveWriteIndex(ret);

        // 如果读取的字节数等于最大可读字节数，那么说明可能还没有读完
        if (ret == readMax)
        {
          // 因为现在没有读取数据的空间了，所以下次进入循环时会进行扩容，直接continue就行
          continue;
        }
        // 这时候表示读完数据了
        else if (ret < readMax)
        {
          isReadAll = true;
          break;
        }
      }
      // 对端关闭了
      else if (ret == 0)
      {
        DEBUGLOG("the client [%s] is closed, clientfd=[%d]", myClientAddr->ToString().c_str(), myFD);
        isClosed = true;
        break;
      }
      // 已经读完数据
      else if (ret == -1 && errno == EAGAIN)
      {
        isReadAll = true;
        break;
      }
    }

    if (isClosed)
    {
      INFOLOG("client closed, client address=[%s], clientfd=[%d]", myClientAddr->ToString().c_str(), myFD);
      ClearConnection();
      return;
    }

    if (!isReadAll)
    {
      ERRORLOG("not read all data!");
    }

    // TODO：RPC协议解析
    OnExcute();
  }

  // 执行回调函数
  void TcpConnection::OnExcute()
  {
    // 从客户端获取RPC请求，执行业务逻辑，获取RPC响应，再把响应发送到客户端
    // 这里先写一个echo做示例
    std::vector<char> tmp;
    int size = myRecvBuffer->ReadRemain();
    tmp.resize(size);
    myRecvBuffer->ReadFromBuffer(tmp, size);

    std::string msg;
    for (size_t i = 0; i < size; i++)
    {
      msg += tmp[i];
    }

    INFOLOG("success get request[%s] from client[%s]", msg.c_str(), myClientAddr->ToString().c_str());

    mySendBuffer->WriteToBuffer(msg.c_str(), msg.length());

    // 将消息发送到客户端
    myFDEvent->Listen(FDEvent::OUT_EVENT, std::bind(&TcpConnection::OnWrite, this));
    myEventLoop->AddEpollEvent(myFDEvent);
  }

  // 写发送缓冲区回调函数
  void TcpConnection::OnWrite()
  {
    // 将当前发送缓冲区mySendBuffer里的数据写入到socket缓冲区，然后发送给客户端
    if (myState != Connected)
    {
      ERRORLOG("OnWrite error! client has already disconnected, addr=[%s]", myClientAddr->ToString().c_str());
      return;
    }

    // 写完标志
    bool isWriteAll = false;
    while (1)
    {
      // 先判断还有没有数据能写
      if (mySendBuffer->ReadRemain() == 0)
      {
        DEBUGLOG("no data need to send to client [%s]", myClientAddr->ToString().c_str());
        isWriteAll = true;
        break;
      }

      int writeMax = mySendBuffer->ReadRemain();

      int readIndex = mySendBuffer->GetReadIndex();

      int ret = write(myFD, &(mySendBuffer->myBuffer[readIndex]), writeMax);

      // 表示数据写入成功
      if (ret >= writeMax)
      {
        // 要先移动读指针
        mySendBuffer->MoveReadIndex(ret);

        DEBUGLOG("all the data has been sent to client [%s]", myClientAddr->ToString().c_str());
        isWriteAll = true;
        break;
      }
      else if (ret == -1 && errno == EAGAIN)
      {
        // socket的发送缓冲区已满，不能再发送。等下次fd可写的时候再发送数据
        ERRORLOG("write data error, error=EAGAIN and ret=-1");
        break;
      }
    }

    // 写完之后要将监听写事件取消，才不会一直在写
    if (isWriteAll)
    {
      myFDEvent->CancelEvent(FDEvent::OUT_EVENT);
      myEventLoop->AddEpollEvent(myFDEvent);
    }
  }

  // 设置当前连接的状态
  void TcpConnection::SetState(const TcpState state)
  {
    myState = state;
  }

  // 获取当前连接的状态
  TcpState TcpConnection::GetState()
  {
    return myState;
  }

  void TcpConnection::ClearConnection()
  {
    // 处理一些关闭连接后的清理动作
    if (myState == Closed)
    {
      return;
    }

    myFDEvent->CancelEvent(FDEvent::IN_EVENT);
    myFDEvent->CancelEvent(FDEvent::OUT_EVENT);

    myEventLoop->DeleteEpollEvent(myFDEvent);

    myState = Closed;
  }

  // 服务器主动关闭连接
  void TcpConnection::Shutdown()
  {
    if (myState == Closed || myState == NotConnected)
    {
      return;
    }

    // 将状态设置为半连接
    myState = HalfClosing;

    // 调用shutdown关闭读写，则服务器不会再对这个fd进行读写操作，但是客户端还是可以往服务器发送数据
    // 服务器发送FIN，触发四次挥手的第一个阶段
    // 当fd的可读时间读到0时，说明对端发送FIN，关闭了连接
    // 主动发送FIN的一端进入FIN_WAIT，接收FIN的一端进入TIME_WAIT
    ::shutdown(myFD, SHUT_RDWR);
  }

  // 获取当前连接的fd
  int TcpConnection::GetFD()
  {
    return myFD;
  }

  // 获取服务端地址
  IPNetAddr::myNetAddrPtr TcpConnection::GetServerAddr()
  {
    return myServerAddr;
  }

  // 获取客户端地址
  IPNetAddr::myNetAddrPtr TcpConnection::GetClientAddr()
  {
    return myClientAddr;
  }

  // 设置连接的类型
  void TcpConnection::SetConnectionType(TcpConnectionType type)
  {
    myConnectionType = type;
  }
}
