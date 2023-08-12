/* ************************************************************************
> File Name:     tcp_client.h
> Author:        Luncles
> 功能:          TCP的客户端，会自己创建一个eventloop，所以要注意去启动loop函数，而且客户端是没有io线程的
> Created Time:  2023年08月07日 星期一 18时19分20秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_TCP_TCP_CLIENT_H
#define MYROCKETRPC_NET_TCP_TCP_CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include "net_addr.h"
#include "../eventloop.h"
#include "../fd_event.h"
#include "tcp_connection.h"
#include "../coder/abstract_protocol.h"

namespace myRocket
{
  class TcpClient
  {
  public:
    using myTcpClient = std::shared_ptr<TcpClient>;

    TcpClient(NetAddr::myNetAddrPtr serverAddress);

    ~TcpClient();

    // 异步地进行connect
    // 借助epoll，如果connect完成了，会触发回调函数
    void ConnectServer(std::function<void()> ConnectCallBack);

    // 异步地发送数据
    // 借助epoll，如果发送成功，会触发回调函数 void WriteCallBack(AbstractProtocol::myAbstractProtocolPtr message)
    void WriteMessage(AbstractProtocol::myAbstractProtocolPtr message, std::function<void(AbstractProtocol::myAbstractProtocolPtr)> WriteCallBack);

    // 异步地接收数据
    // 借助epoll，如果接收成功，会触发回调函数 void ReadCallBack(AbstractProtocol::myAbstractProtocolPtr message)
    void ReadMessage(const std::string &message, std::function<void(AbstractProtocol::myAbstractProtocolPtr)> ReadCallBack);

    // 关闭客户端的eventloop
    void Stop();

    // 获取本地地址
    IPNetAddr::myNetAddrPtr GetLocalAddress();

    // 获取服务器地址
    IPNetAddr::myNetAddrPtr GetServerAddress();

    // 获取连接错误消息码
    int GetConnectErrorCode();

    // 获取连接错误消息
    std::string GetConnectErrorInfo();

    // 初始化本地地址
    void InitLocalAddr();

  private:
    // 连接的本地地址
    IPNetAddr::myNetAddrPtr myLocalAddr;

    // 对端服务器的地址
    IPNetAddr::myNetAddrPtr myServerAddr;

    // 在本地创建一个eventloop
    EventLoop *myEventLoop{nullptr};

    // 直接在本地创建fd，描述符信息
    int myFD{-1};
    FDEvent *myFDEvent{nullptr};

    // tcp连接
    TcpConnection::myTcpConnectionPtr myTcpConnection;

    // 连接错误消息码
    int myConnectErrorCode{0};

    // 连接错误信息
    std::string myConnectErrorInfo;
  };

}

#endif
