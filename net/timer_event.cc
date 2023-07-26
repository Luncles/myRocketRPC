/* ************************************************************************
> File Name:     timer_event.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年07月24日 星期一 20时18分51秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "timer_event.h"
#include "/home/luncles/myRocketRPC/common/log.h"
#include "/home/luncles/myRocketRPC/common/util.h"

namespace myRocket
{
  TimerEvent::TimerEvent(int interval, bool isRepeated, std::function<void()> CallBack) : myInterval(interval), myIsRepeated(isRepeated), myTask(CallBack)
  {
    ResetArriveTime();
  }

  void TimerEvent::ResetArriveTime()
  {
    myArriveTime = GetNowMS() + myInterval;
    DEBUGLOG("success create timer event, will excute at [%d]", myArriveTime);
  }
}