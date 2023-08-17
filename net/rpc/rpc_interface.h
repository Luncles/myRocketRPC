/* ************************************************************************
> File Name:     rpc_interface.h
> Author:        Luncles
> 功能:          RPC接口基类
> Created Time:  2023年08月16日 星期三 17时09分26秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_RPC_RPC_INTERFACE_H
#define MYROCKETRPC_NET_RPC_RPC_INTERFACE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include <google/protobuf/message.h>
#include "myRocketRPC/net/rpc/rpc_controller.h"
#include "myRocketRPC/net/rpc/rpc_closure.h"

namespace myRocketRPC
{
  class RpcClosure;

  class RpcInterface : public std::enable_shared_from_this<RpcInterface>
  {
  public:
    RpcInterface(const google::protobuf::Message *req, google::protobuf::Message *rsp, RpcClosure *done, RpcController *controller);

    virtual ~RpcInterface();

    // 对客户端的回复
    void reply();

    // 释放资源
    void destroy();

    // 给该接口分配一个回调函数对象
    std::shared_ptr<RpcClosure> newRpcClosure(std::function<void()> &cb);

    // 核心业务处理方法
    virtual void run() = 0;

    // 设置错误码和错误信息
    virtual void setError(int code, const std::string &errInfo) = 0;

  protected:
    const google::protobuf::Message *m_req_base{nullptr};

    google::protobuf::Message *m_rsp_base{nullptr};

    RpcClosure *m_done{nullptr};

    RpcController *m_controller{nullptr};

  private:
  };
}

#endif