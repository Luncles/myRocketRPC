/* ************************************************************************
> File Name:     timer_event.h
> Author:        Luncles
> 功能:          定时器事件类
> Created Time:  2023年07月24日 星期一 20时18分48秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_TIMER_EVENT_H
#define MYROCKETRPC_NET_TIMER_EVENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <functional>
#include <memory>

namespace myRocket
{
  class TimerEvent
  {
  public:
    using myTimerEventPtr = std::shared_ptr<TimerEvent>;

    TimerEvent(int interval, bool isRepeated, std::function<void()> CallBack);

    int64_t GetArriveTime() const
    {
      return myArriveTime;
    }

    void SetCanceledTimer(bool value)
    {
      myIsCanceled = value;
    }

    bool IsCanceled()
    {
      return myIsCanceled;
    }

    bool IsRepeated()
    {
      return myIsRepeated;
    }

    std::function<void()> GetCallBack()
    {
      return myTask;
    }

    // 重置定时器定时时间
    void ResetArriveTime();

  private:
    int64_t myArriveTime;     // 定时时间:ms
    int64_t myInterval;       // 时间间隔:ms
    bool myIsRepeated{false}; // 是否要重复定时器
    bool myIsCanceled{false}; // 是否要取消定时器

    std::function<void()> myTask;
  };
}
#endif
