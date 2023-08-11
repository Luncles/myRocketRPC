/* ************************************************************************
> File Name:     rpc_dispatcher.h
> Author:        Luncles
> 功能:          RPC对话器
> Created Time:  2023年08月10日 星期四 16时05分06秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_RPC_RPC_DISPATHCER_H
#define MYROCKETRPC_NET_RPC_RPC_DISPATHCER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include <map>
#include <google/protobuf/service.h>

#include "../coder/abstract_protocol.h"
#include "../coder/tinypb_protocol.h"
#include "../tcp/tcp_connection.h"

namespace myRocket
{
  // class TcpConnection;

  class RpcDispatcher
  {
  public:
    static RpcDispatcher *GetRpcDispatcher();

  public:
    using protobuf_service_s_ptr = std::shared_ptr<google::protobuf::Service>;

    void Dispatch(AbstractProtocol::myAbstractProtocolPtr request, AbstractProtocol::myAbstractProtocolPtr response, TcpConnection *connection);

    // 注册一个OrderService对象
    void RegisterService(protobuf_service_s_ptr service);

    // 设置错误信息
    void SetTinyPBError(std::shared_ptr<TinyProtocol> msg, int32_t errorCode, std::string errorInfo);

  private:
    // 传进来的fullname是service.method，所以需要进行拆分
    bool ParseServiceFullName(const std::string &fullName, std::string &serviceName, std::string &methodName);

  private:
    std::map<std::string, protobuf_service_s_ptr> myServiceMap;
  };
}
#endif