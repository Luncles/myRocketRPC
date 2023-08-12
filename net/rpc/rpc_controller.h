/* ************************************************************************
> File Name:     rpc_controller.h
> Author:        Luncles
> 功能:          提供一些方法来获取rpc调用的结果
> Created Time:  2023年08月10日 星期四 20时11分42秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_RPC_RPC_CONTROLLER_H
#define MYROCKETRPC_NET_RPC_RPC_CONTROLLER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "../tcp/net_addr.h"
#include "../../common/log.h"

namespace myRocket
{
  class RpcController : public google::protobuf::RpcController
  {
  public:
    RpcController()
    {
      INFOLOG("RpcController");
    }
    ~RpcController()
    {
      INFOLOG("~RpcController");
    }
    /*************************** 派生类自定义函数 ***********************************/
    void SetFinished(bool value);

    bool Finished();

    int GetTimeout();

    void SetTimeout(int timeout);

    NetAddr::myNetAddrPtr GetLocalAddress();

    NetAddr::myNetAddrPtr GetPeerAddress();

    void SetLocalAddress(NetAddr::myNetAddrPtr address);

    void SetPeerAddress(NetAddr::myNetAddrPtr address);

    void SetMessageID(const std::string &messageID);

    std::string GetMessageID();

    void SetError(int32_t errorCode, const std::string errorInfo);

    int32_t GetErrorCode();

    std::string GetErrorInfo();

    /*************************** 客户端函数 ***********************************/
    // Resets the RpcController to its initial state so that it may be reused in
    // a new call.  Must not be called while an RPC is in progress.
    void Reset();

    bool Failed() const;

    // If Failed() is true, returns a human-readable description of the error.
    std::string ErrorText() const;

    void StartCancel();

    /*************************** 服务端函数 ***********************************/
    void SetFailed(const std::string &reason);

    // If true, indicates that the client canceled the RPC
    bool IsCanceled() const;

    void NotifyOnCancel(google::protobuf::Closure *callback);

  private:
    int32_t myErrorCode{0};
    std::string myErrorInfo;
    std::string myMessageID;

    bool myIsFailed{false};
    bool myIsCanceled{false};
    bool myIsFinished{false};

    NetAddr::myNetAddrPtr myLocalAddress;
    NetAddr::myNetAddrPtr myPeerAddress;

    // 超时时间，ms
    int myTimeout{1000};
  };
}
#endif