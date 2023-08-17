/* ************************************************************************
> File Name:     timer.h
> Author:        Luncles
> 功能:          定时器类，为了统一事件源，继承自FDEvent类
> Created Time:  2023年07月24日 星期一 20时56分40秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_TIMER_H
#define MYROCKETRPC_NET_TIMER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <map>
#include "fd_event.h"
#include "timer_event.h"
#include "myRocketRPC/common/mutex.h"

namespace myRocketRPC
{
  class Timer : public FDEvent
  {
  public:
    Timer();

    ~Timer();

    // 添加定时器任务到定时器表中
    void AddTimerEvent(TimerEvent::myTimerEventPtr timeEvent);

    void DeleteTimerEvent(TimerEvent::myTimerEventPtr timeEvent);

    // 触发定时器事件，统一事件源，当发生IO事件后，eventloop会执行这个事件处理器，执行对应的TimerEvent的回调函数
    void OnTimer();

  private:
    void ResetTimerArriveTime();

  private:
    // 管理定时器容器：multimap，底层是红黑树，会根据触发时间对timer event进行排序
    std::multimap<int64_t, TimerEvent::myTimerEventPtr> myTimerEventMap;
    pMutex myMutex;
  };
} // namespace name

#endif
