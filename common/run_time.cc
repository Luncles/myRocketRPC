/* ************************************************************************
> File Name:     run_time.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月14日 星期一 19时57分48秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "run_time.h"

namespace myRocketRPC
{
  thread_local RunTime *myRunTime = nullptr;
  RunTime *RunTime::GetRunTime()
  {
    if (myRunTime)
    {
      return myRunTime;
    }
    myRunTime = new RunTime();
    return myRunTime;
  }

  RpcInterface *RunTime::GetRpcInterface()
  {
    return myRpcInterface;
  }
}
