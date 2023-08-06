/* ************************************************************************
> File Name:     tcp_server.h
> Author:        Luncles
> 功能:          TCP服务器模块
> Created Time:  2023年08月04日 星期五 16时57分47秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_TCP_TCP_SERVER_H
#define MYROCKETRPC_NET_TCP_TCP_SERVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <set>
#include "tcp_accept.h"
#include "net_addr.h"
#include "/home/luncles/myRocketRPC/net/eventloop.h"
#include "/home/luncles/myRocketRPC/net/io_thread_group.h"
#include "/home/luncles/myRocketRPC/net/fd_event.h"
#include "tcp_connection.h"

namespace myRocket
{
  class TCPServer
  {
  public:
    TCPServer(NetAddr::myNetAddrPtr localAddress);

    ~TCPServer();

    // 启动服务器
    void Start();

  private:
    void InitServer();

    // 当有新客户端连接后需要执行，将客户端地址传给一个io线程进行业务处理
    void OnAccept();

    // 清除已经close的客户端连接
    void ClearClosedClientTimer();

  private:
    TcpAccept::myTcpAcceptPtr myAcceptor; // 服务端连接器

    NetAddr::myNetAddrPtr myLocalAddress; // 本地的监听地址

    EventLoop *myMainEventLoop{nullptr}; // 主线程的eventloop

    IOThreadGroup *myIOThreadGroup{nullptr}; // io线程组，每个io线程有各自的eventloop，相当于subreactor

    FDEvent *myListenFDEvent{nullptr}; // 方便在主线程中获取监听fd

    std::set<TcpConnection::myTcpConnectionPtr> myClientConnection; // 客户端连接集合

    int myClientCount{0}; // 记录连接的客户端数
  };
}

#endif