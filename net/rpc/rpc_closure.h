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
#include "myRocketRPC/common/log.h"
#include "myRocketRPC/common/run_time.h"
#include "myRocketRPC/common/exception.h"
#include "myRocketRPC/net/rpc/rpc_interface.h"

namespace myRocketRPC
{
  class RpcClosure : public google::protobuf::Closure
  {
  public:
    using myRpcInterfacePtr = std::shared_ptr<RpcInterface>;

    RpcClosure(std::function<void()> cb) : myCallBack(cb)
    {
      INFOLOG("RpcClosure");
    }

    RpcClosure(myRpcInterfacePtr interface, std::function<void()> cb) : myCallBack(cb)
    {
      INFOLOG("RpcClosure");
    }

    ~RpcClosure()
    {
      INFOLOG("~RpcClosure");
    }

    void Run() override
    {
      // 更新 runtime 的 RpcInterFace, 这里在执行 cb 的时候，都会以 RpcInterface 找到对应的接口，实现打印 app 日志等
      if (!myRpcinterface)
      {
        RunTime::GetRunTime()->myRpcInterface = myRpcinterface.get();
      }

      try
      {
        if (myCallBack)
        {
          myCallBack();
        }
        if (myRpcinterface)
        {
          myRpcinterface.reset();
        }
      }
      catch (RocketException &e)
      {
        ERRORLOG("RocketException exception[%s], deal handle", e.what());
        e.handle();
        if (myRpcinterface)
        {
          myRpcinterface->setError(e.ErrorCode(), e.ErrorInfo());
          myRpcinterface.reset();
        }
      }
      catch (std::exception &e)
      {
        ERRORLOG("std::exception[%s]", e.what());
        if (myRpcinterface)
        {
          myRpcinterface->setError(-1, "unknown std::exception");
          myRpcinterface.reset();
        }
      }
      catch (...)
      {
        ERRORLOG("Unknown exception");
        if (myRpcinterface)
        {
          myRpcinterface->setError(-1, "unknown exception");
          myRpcinterface.reset();
        }
      }
    }

  private:
    std::function<void()> myCallBack{nullptr};
    myRpcInterfacePtr myRpcinterface{nullptr};
  };
}

#endif