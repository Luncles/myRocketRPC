/* ************************************************************************
> File Name:     run_time.h
> Author:        Luncles
> 功能:          获取一些在线程运行过程中的变量
> Created Time:  2023年08月14日 星期一 19时57分44秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_COMMON_RUN_TIME_H
#define MYROCKETRPC_COMMON_RUN_TIME_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

namespace myRocket
{
  class RunTime
  {
  public:
  public:
    // 全局的RunTime
    static RunTime *GetRunTime();

  public:
    std::string messageID;
    std::string methodName;
  };
}

#endif