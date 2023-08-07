/* ************************************************************************
> File Name:     tcp_connection.h
> Author:        Luncles
> 功能:          客户端与RPC服务器之间的数据中转，也是业务的处理中心，数据在这里进行处理
> Created Time:  2023年08月05日 星期六 19时39分40秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_TCP_TCP_CONNECTION_H
#define MYROCKETRPC_NET_TCP_TCP_CONNECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include "../eventloop.h"
#include "../io_thread.h"
#include "../fd_event.h"
#include "net_addr.h"
#include "tcp_buffer.h"

namespace myRocket
{
  // 当前TCP连接所处的状态
  enum TcpState
  {
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 4,
  };

  // TcpConnection即可以被服务端调用，也可以被客户端调用，代表了一个连接
  enum TcpConnectionType
  {
    // 调用tcp connection的主体
    TcpConnectionByServer = 1, // 服务端调用，代表跟对端客户端的连接
    TcpConnectionByClient = 2, // 客户端调用，代表跟对端服务端的连接
  };

  class TcpConnection
  {
  public:
    using myTcpConnectionPtr = std::shared_ptr<TcpConnection>;

  public:
    TcpConnection(EventLoop *eventloop, int fd, int bufferSize, IPNetAddr::myNetAddrPtr serverAddr, IPNetAddr::myNetAddrPtr clientAddr, TcpConnectionType type = TcpConnectionByServer);

    ~TcpConnection();

    // 读接收缓冲区回调函数
    void OnRead();

    // 执行回调函数
    void OnExcute();

    // 写发送缓冲区回调函数
    void OnWrite();

    // 设置当前连接的状态
    void SetState(const TcpState state);

    // 获取当前连接的状态
    TcpState GetState();

    // 服务器主动关闭连接
    void Shutdown();

    // 获取当前连接的fd
    int GetFD();

    // 获取服务端地址
    IPNetAddr::myNetAddrPtr GetServerAddr();

    // 获取客户端地址
    IPNetAddr::myNetAddrPtr GetClientAddr();

    // 客户端关闭后清理现场
    void ClearConnection();

    // 设置连接的类型
    void SetConnectionType(TcpConnectionType type);

  private:
    // 代表持有当前连接的io线程
    EventLoop *myEventLoop{nullptr};

    // 服务端地址
    IPNetAddr::myNetAddrPtr myServerAddr;

    // 客户端地址
    IPNetAddr::myNetAddrPtr myClientAddr;

    // 发送缓冲区
    TCPBuffer::myTCPBufferPtr mySendBuffer;

    // 接收缓冲区
    TCPBuffer::myTCPBufferPtr myRecvBuffer;

    FDEvent *myFDEvent{nullptr};

    TcpState myState;

    TcpConnectionType myConnectionType{TcpConnectionByServer};

    int myFD{0};
  };
}
#endif
