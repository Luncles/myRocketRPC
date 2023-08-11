/* ************************************************************************
> File Name:     rpc_controller.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月10日 星期四 20时11分47秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "rpc_controller.h"

namespace myRocket
{
  // reset函数在这里就是把各个成员变量还原
  void RpcController::Reset()
  {
    myErrorCode = 0;
    myErrorInfo = "";
    myMessageID = "";

    myIsFailed = false;
    myIsCanceled = false;
    myIsFinished = false;

    myLocalAddress = nullptr;
    myPeerAddress = nullptr;

    // 超时时间，ms
    myTimeout = 1000;
  }

  bool RpcController::Failed() const
  {
    return myIsFailed;
  }

  // If Failed() is true, returns a human-readable description of the error.
  std::string RpcController::ErrorText() const
  {
    return myErrorInfo;
  }

  void RpcController::StartCancel()
  {
    myIsCanceled = true;
    myIsFailed = true;
    SetFinished(true);
  }

  bool RpcController::IsCanceled() const
  {
    return myIsCanceled;
  }

  void RpcController::SetFailed(const std::string &reason)
  {
    myIsFailed = true;
    myErrorInfo = reason;
  }

  void RpcController::NotifyOnCancel(google::protobuf::Closure *callback)
  {
  }

  void RpcController::SetFinished(bool value)
  {
    myIsFinished = value;
  }

  bool RpcController::Finished()
  {
    return myIsFinished;
  }

  int RpcController::GetTimeout()
  {
    return myTimeout;
  }

  void RpcController::SetTimeout(int timeout)
  {
    myTimeout = timeout;
  }

  NetAddr::myNetAddrPtr RpcController::GetLocalAddress()
  {
    return myLocalAddress;
  }

  NetAddr::myNetAddrPtr RpcController::GetPeerAddress()
  {
    myPeerAddress;
  }

  void RpcController::SetLocalAddress(NetAddr::myNetAddrPtr address)
  {
    myLocalAddress = address;
  }

  void RpcController::SetPeerAddress(NetAddr::myNetAddrPtr address)
  {
    myPeerAddress = address;
  }

  void RpcController::SetMessageID(const std::string &messageID)
  {
    myMessageID = messageID;
  }

  std::string RpcController::GetMessageID()
  {
    return myMessageID;
  }

  void RpcController::SetError(int32_t errorCode, const std::string errorInfo)
  {
    myErrorCode = errorCode;
    myErrorInfo = errorInfo;
    myIsFailed = true;
  }

  int32_t RpcController::GetErrorCode()
  {
    return myErrorCode;
  }

  std::string RpcController::GetErrorInfo()
  {
    return myErrorInfo;
  }
}
