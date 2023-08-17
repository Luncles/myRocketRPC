/* ************************************************************************
> File Name:     exception.h
> Author:        Luncles
> 功能:          异常处理虚基类，需要在用户rpc调用里自定义
> Created Time:  2023年08月16日 星期三 14时42分39秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_COMMON_EXCEPTION_H
#define MYROCKETRPC_COMMON_EXCEPTION_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <exception>
#include <string>

namespace myRocketRPC
{
  class RocketException : public std::exception
  {
  public:
    RocketException(int errorCode, const std::string &errorInfo) : myErrorCode(errorCode), myErrorInfo(errorInfo) {}

    // 异常处理
    // 当捕获到RocketException及其子类对象的异常时，会执行该函数
    virtual void handle() = 0;

    virtual ~RocketException() {}

    int ErrorCode()
    {
      return myErrorCode;
    }

    std::string ErrorInfo()
    {
      return myErrorInfo;
    }

  protected:
    int myErrorCode{0};

    std::string myErrorInfo;
  };
}

#endif