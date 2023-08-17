/* ************************************************************************
> File Name:     rpc_interface.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月16日 星期三 17时09分30秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "myRocketRPC/common/log.h"
#include "myRocketRPC/net/rpc/rpc_closure.h"
#include "myRocketRPC/net/rpc/rpc_controller.h"
#include "myRocketRPC/net/rpc/rpc_interface.h"

namespace myRocketRPC
{
  RpcInterface::RpcInterface(const google::protobuf::Message *req, google::protobuf::Message *rsp, RpcClosure *done, RpcController *controller) : m_req_base(req), m_rsp_base(rsp), m_done(done), m_controller(controller)
  {
    INFOLOG("RpcInterface");
  }

  RpcInterface::~RpcInterface()
  {
    INFOLOG("~RpcInterface");

    reply();

    destroy();
  }

  // 对客户端的回复
  void RpcInterface::reply()
  {
    // reply to client
    // you should call is when you wan to set response back
    // it means this rpc method done
    if (m_done)
    {
      m_done->Run();
    }
  }

  // 释放资源
  void RpcInterface::destroy()
  {
    if (m_req_base)
    {
      delete m_req_base;
      m_req_base = nullptr;
    }

    if (m_rsp_base)
    {
      delete m_rsp_base;
      m_rsp_base = nullptr;
    }

    if (m_done)
    {
      delete m_done;
      m_done = nullptr;
    }

    if (m_controller)
    {
      delete m_controller;
      m_controller = nullptr;
    }
  }

  // 给该接口分配一个回调函数对象
  std::shared_ptr<RpcClosure> RpcInterface::newRpcClosure(std::function<void()> &cb)
  {
    return std::make_shared<RpcClosure>(shared_from_this(), cb);
  }
}