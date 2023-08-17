/* ************************************************************************
> File Name:     msg_id_util.h
> Author:        Luncles
> 功能:          生成message id的类
> Created Time:  2023年08月11日 星期五 16时33分34秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_COMMON_MSGID_UTIL_H
#define MYROCKETRPC_COMMON_MSGID_UTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

namespace myRocketRPC
{
  class MsgIDUtil
  {
  public:
    static std::string MsgIDGenerator();

  private:
  };
}
#endif