/* ************************************************************************
> File Name:     rpc_closure.h
> Author:        Luncles
> 功能:          回调函数
> Created Time:  2023年08月10日 星期四 21时59分32秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_RPC_RPC_CLOSURE_H
#define MYROCKETRPC_NET_RPC_RPC_CLOSURE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <functional>
#include <google/protobuf/stubs/callback.h>
#include "../../common/log.h"

namespace myRocket
{
  class RpcClosure : public google::protobuf::Closure
  {
  public:
    RpcClosure()
    {
      INFOLOG("RpcClosure");
    }
    ~RpcClosure()
    {
      INFOLOG("~RpcClosure");
    }

    void Run() override
    {
      if (myCallBack)
      {
        myCallBack();
      }
    }

  private:
    std::function<void()> myCallBack{nullptr};
  };
}

#endif