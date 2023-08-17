/* ************************************************************************
> File Name:     timer.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年07月24日 星期一 20时56分46秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <vector>
#include <functional>
#include "timer.h"
#include "myRocketRPC/common/log.h"
#include "myRocketRPC/common/util.h"

namespace myRocketRPC
{
  Timer::Timer() : FDEvent()
  {
    /*
    调用了 timerfd_create 函数来创建一个新的定时器对象，并将返回的文件描述符存储在 myFD 变量中。
    timerfd_create 函数接受两个参数。第一个参数 CLOCK_MONOTONIC 指定了用于标记定时器进度的时钟。CLOCK_MONOTONIC 是一个不可设置的时钟，它不受系统时钟中的非连续改变的影响（例如，手动改变系统时间）。
    第二个参数是一个标志位，它可以按位或来改变 timerfd_create 的行为。在这个例子中，我们使用了两个标志：TFD_NONBLOCK 和 TFD_CLOEXEC。
    TFD_NONBLOCK 标志为新打开的文件描述符设置了 O_NONBLOCK 选项。这意味着对这个文件描述符执行的读写操作都是非阻塞的。如果读写操作不能立即完成，它们会立即返回而不是阻塞等待。
    TFD_CLOEXEC 标志设置了新文件描述符上的 close-on-exec (FD_CLOEXEC) 标志。这意味着当执行一个新的程序时，这个文件描述符会被自动关闭。这样可以防止文件描述符泄漏到子进程中。
    */
    myFD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    DEBUGLOG("create timer fd = [%d]", myFD);

    // std::bind(OnTimer, this)：将无参成员函数OnTimer和类对象绑定在一起，这样可以在不定义对象的情况下调用成员函数
    // 发生读事件时就会调用回调函数OnTimer()，再去执行每个到时定时器对应的事件处理器（回调函数）
    Listen(FDEvent::IN_EVENT, std::bind(&Timer::OnTimer, this));
  }

  // multimap和智能指针对自动释放资源，不用做什么
  Timer::~Timer()
  {
  }

  void Timer::AddTimerEvent(TimerEvent::myTimerEventPtr timerEvent)
  {
    // 记录要插入的定时器是否需要重新设定触发时间
    bool needResetTimerfd = false;

    ScopeMutex<pMutex> lock(myMutex);

    if (myTimerEventMap.empty())
    {
      needResetTimerfd = true;
    }
    else
    {
      auto it = myTimerEventMap.begin();
      // 如果当前要插入的定时器的触发时间比当前定时器容器最小的触发时间还小，那么说明当前要插入的定时器是不会被触发的，所以需要“拯救”要插入的定时器——重新设定触发时间
      if ((*it).second->GetArriveTime() > timerEvent->GetArriveTime())
      {
        needResetTimerfd = true;
      }
    }
    myTimerEventMap.emplace(std::make_pair(timerEvent->GetArriveTime(), timerEvent));
    lock.unlock();

    if (needResetTimerfd)
    {
      ResetTimerArriveTime();
    }
  }

  void Timer::DeleteTimerEvent(TimerEvent::myTimerEventPtr timerEvent)
  {
    // 首先要取消这个定时器
    timerEvent->SetCanceledTimer(true);

    ScopeMutex<pMutex> lock(myMutex);

    // 找到一组与timerEvent触发时间一样的定时器
    auto begin = myTimerEventMap.lower_bound(timerEvent->GetArriveTime());
    auto end = myTimerEventMap.upper_bound(timerEvent->GetArriveTime());

    // 从定时器组中找出要删除的定时器
    auto it = begin;
    for (; it != end; it++)
    {
      if ((*it).second == timerEvent)
      {
        break;
      }
    }

    if (it != end)
    {
      myTimerEventMap.erase(it);
    }
    lock.unlock();

    DEBUGLOG("success delete TimerEvent at arrive time [%lld], fd=[%d]", timerEvent->GetArriveTime(), myFD);
  }

  void Timer::ResetTimerArriveTime()
  {
    ScopeMutex<pMutex> lock(myMutex);
    auto tmp = myTimerEventMap;
    lock.unlock();

    if (tmp.empty())
    {
      return;
    }

    int64_t now = GetNowMS();
    auto it = tmp.begin();
    int64_t interval = 0;
    // 最小触发时间还没到
    if ((*it).second->GetArriveTime() > now)
    {
      interval = (*it).second->GetArriveTime() - now;
    }
    // 已经过了最小触发时间了，需要立即执行过了的任务
    else
    {
      interval = 100;
    }

    struct timespec ts;
    memset(&ts, 0, sizeof(ts));
    ts.tv_sec = interval / 1000;
    ts.tv_nsec = (interval % 1000) * 1000000;

    struct itimerspec its;
    memset(&its, 0, sizeof(its));
    its.it_value = ts;

    int result = timerfd_settime(myFD, 0, &its, nullptr); // 重新设置触发时间
    if (result != 0)
    {
      ERRORLOG("timerfd_settime error, errno=[%d], error=[%s]", errno, strerror(errno));
    }
    DEBUGLOG("timer reset to [%lld]", now + interval);
  }

  void Timer::OnTimer()
  {
    DEBUGLOG("on timer!");

    // 因为是水平触发的，所以需要先把触发消息从读缓冲区读出，防止一直触发读事件
    char buf[8];
    while (1)
    {
      // 还没读到文件末尾，但是暂时读不到数据，因为是非阻塞状态，说明读缓冲区的数据都读完了
      if (read(myFD, buf, 8) == -1 && errno == EAGAIN)
      {
        break;
      }
    }

    // 执行定时任务
    int64_t now = GetNowMS();

    // 用来保存执行完但是不取消的定时器
    std::vector<TimerEvent::myTimerEventPtr> tmpTEPVec;
    // 保存需要执行的定时任务，减少锁的粒度
    std::vector<std::pair<int64_t, std::function<void()>>> tmpTask;

    // 接下来检查那些定时器任务超时了，超时了的就需要执行回调函数
    ScopeMutex<pMutex> lock(myMutex);
    auto it = myTimerEventMap.begin();
    for (; it != myTimerEventMap.end(); ++it)
    {
      // 这些任务已经超时
      if ((*it).first <= now)
      {
        // 如果是不取消的任务，就要另外储存起来，然后再插入到定时器表中，然后执行其回调函数
        if (!(*it).second->IsCanceled())
        {
          tmpTEPVec.push_back((*it).second);
          tmpTask.push_back(std::make_pair((*it).first, (*it).second->GetCallBack()));
        }
      }
      else
      {
        break;
      }
    }

    // 从定时器表中删除那些超时的，在后面再进行插入
    myTimerEventMap.erase(myTimerEventMap.begin(), it);
    lock.unlock();

    // 将那些删除了，而且是需要重复的定时器再插入到定时器表中，因为multimap是根据插入时的关键字进行排序的，这时候才能对那些执行过的重新进行排序
    for (auto it = tmpTEPVec.begin(); it != tmpTEPVec.end(); ++it)
    {
      if ((*it)->IsRepeated())
      {
        // 调整arrive time
        (*it)->ResetArriveTime();
        AddTimerEvent(*it);
      }
    }

    // 保险起见，检查有没有遗漏的定时器任务---最后修改，这一句必须加，不然有些定时任务只会触发一次
    ResetTimerArriveTime();

    // 接下来执行超时任务的回调函数
    for (auto it : tmpTask)
    {
      if (it.second)
      {
        it.second();
      }
    }
  }
}