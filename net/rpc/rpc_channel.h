/* ************************************************************************
> File Name:     rpc_channel.h
> Author:        Luncles
> 功能:           客户端和服务端通信的封装
> Created Time:  2023年08月11日 星期五 16时57分02秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_RPC_RPC_CHANNEL_H
#define MYROCKETRPC_NET_RPC_RPC_CHANNEL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "../tcp/net_addr.h"
#include "../tcp/tcp_client.h"

namespace myRocket
{
  class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel>
  {
  public:
    using myRpcChannelPtr = std::shared_ptr<RpcChannel>;
    using myControllerPtr = std::shared_ptr<google::protobuf::RpcController>;
    using myMessagePtr = std::shared_ptr<google::protobuf::Message>;
    using myClosurePtr = std::shared_ptr<google::protobuf::Closure>;

  public:
    RpcChannel(NetAddr::myNetAddrPtr peerAddr);

    ~RpcChannel();

    // 调用远程服务的方法
    // Call the given method of the remote service.  The signature of this
    // procedure looks the same as Service::CallMethod(), but the requirements
    // are less strict in one important way:  the request and response objects
    // need not be of any specific class as long as their descriptors are
    // method->input_type() and method->output_type().
    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                    google::protobuf::Message *response, google::protobuf::Closure *done);

    TcpClient *GetTcpClient();

    google::protobuf::RpcController *GetController();

    google::protobuf::Message *GetRequest();

    google::protobuf::Message *GetResponse();

    google::protobuf::Closure *GetClosure();

    // 获取proto资源
    void Init(myControllerPtr controller, myMessagePtr request, myMessagePtr response, myClosurePtr done);

  private:
    // 执行的回调函数
    void myCallBack();

  private:
    NetAddr::myNetAddrPtr myPeerAddr{nullptr};
    NetAddr::myNetAddrPtr myLocalAddr{nullptr};

    // 定义这些智能指针都是为了保证在CallMethod函数执行过程中这些资源不会被析构掉
    myControllerPtr myController{nullptr};
    myMessagePtr myRequest{nullptr};
    myMessagePtr myResponse{nullptr};
    myClosurePtr myClosure{nullptr};

    TcpClient::myTcpClient myTcpClient{nullptr};

    bool myIsInit{false}; // 表示channel是否被初始化
  };
}

#endif