/* ************************************************************************
> File Name:     eventloop.cc
> Author:        Luncles
> 功能：          Reactor模式：主线程
> Created Time:  Sat 15 Jul 2023 10:35:08 PM CST
> Description:
 ************************************************************************/

#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>

#include "myRocketRPC/common/log.h"
#include "myRocketRPC/common/util.h"
#include "myRocketRPC/common/mutex.h"
#include "eventloop.h"

// 将fdEvent事件添加到epoll事件表中
#define ADD_TO_EPOLL()                                                                                             \
  int op = EPOLL_CTL_ADD;                                                                                          \
  if (myListenfds.find(fdEvent->GetFD()) != myListenfds.end())                                                     \
  {                                                                                                                \
    op = EPOLL_CTL_MOD;                                                                                            \
  }                                                                                                                \
  struct epoll_event tmp = fdEvent->GetEpollEvent();                                                               \
  INFOLOG("epoll_event.events = %d", (int)tmp.events);                                                             \
  int result = epoll_ctl(myEpollfd, op, fdEvent->GetFD(), &tmp);                                                   \
  if (result == -1)                                                                                                \
  {                                                                                                                \
    ERRORLOG("failed to epoll_ctl when add fd[%d], errno=%d, error=%s", fdEvent->GetFD(), errno, strerror(errno)); \
  }                                                                                                                \
  myListenfds.insert(fdEvent->GetFD());                                                                            \
  DEBUGLOG("add event success, fd[%d]", fdEvent->GetFD());

// 从epoll事件表中删除事件fdEvent
#define DELETE_FROM_EPOILL()                                                                                          \
  auto it = myListenfds.find(fdEvent->GetFD());                                                                       \
  if (it == myListenfds.end())                                                                                        \
  {                                                                                                                   \
    return;                                                                                                           \
  }                                                                                                                   \
  int op = EPOLL_CTL_DEL;                                                                                             \
  int result = epoll_ctl(myEpollfd, op, fdEvent->GetFD(), nullptr);                                                   \
  if (result == -1)                                                                                                   \
  {                                                                                                                   \
    ERRORLOG("failed to epoll_ctl when delete fd[%d], errno=%d, error=%s", fdEvent->GetFD(), errno, strerror(errno)); \
  }                                                                                                                   \
  myListenfds.erase(fdEvent->GetFD());                                                                                \
  DEBUGLOG("delete event success, fd[%d]", fdEvent->GetFD());

namespace myRocketRPC
{
  // 创建具有线程周期的静态变量，在线程开始的时候被生成，在线程结束的时候被销毁，并且每一个线程中都有独立的变量实例
  static thread_local EventLoop *currentEventloop = nullptr;
  // epoll允许的最大事件数
  static int globalEpollMaxEvent = 10;
  // epoll_wait超时时间
  static int globalEpollMaxTimeout = 10000;

  EventLoop::EventLoop()
  {
    if (currentEventloop != nullptr)
    {
      ERRORLOG("failed to create event loop, this thread has created event loop");
      exit(0);
    }

    myThreadID = GetThreadId();
    myEpollfd = epoll_create(1);
    if (myEpollfd == -1)
    {
      INFOLOG("failed to create event loop, epoll_create error, error info[%d]\n",
              errno);
      exit(0);
    }

    // 初始化唤醒fd事件
    InitWakeUpFdEvent();

    // 初始化定时器
    InitTimer();

    INFOLOG("success create event loop in thread %d\n", myThreadID);
    currentEventloop = this;
  }

  EventLoop::~EventLoop()
  {
    close(myEpollfd);
    if (myWakeUpFdEvent != nullptr)
    {
      delete myWakeUpFdEvent;
      myWakeUpFdEvent = nullptr;
    }

    if (myTimer != nullptr)
    {
      delete myTimer;
      myTimer = nullptr;
    }
  }

  void EventLoop::Loop()
  {
    myIsLooping = true;
    while (!myStopFlag)
    {
      // 加锁，尽量减少锁的粒度
      ScopeMutex<pMutex> plock(myMutex);
      std::queue<std::function<void()>> tmpTask;
      myPendingTask.swap(tmpTask);
      plock.unlock();

      while (!tmpTask.empty())
      {
        std::function<void()> TaskCallBack = tmpTask.front();
        tmpTask.pop();
        if (TaskCallBack)
        {
          TaskCallBack();
        }
      }

      // 这里添加定时器任务的逻辑
      // 1、怎么判断一个定时任务需要执行？（需要有一个arrive_time）
      // 2、当arrive_time到时，如何让处于阻塞的epoll_wait返回

      int timeout = globalEpollMaxTimeout;                 // epoll_wait超时时间
      struct epoll_event resultEvent[globalEpollMaxEvent]; // 保存epoll_wait返回的事件
      // DEBUGLOG("now begin to epoll_wait");
      int eventNumber = epoll_wait(myEpollfd, resultEvent, globalEpollMaxEvent, timeout);
      // DEBUGLOG("now end epoll_wait, eventNumber = %d", eventNumber);

      // epoll失败，返回-1
      if (eventNumber < 0)
      {
        ERRORLOG("epoll_wait error, errno=%d, error=%s", errno, strerror(errno));
      }
      else
      {
        // 处理epoll_wait返回的事件
        for (int i = 0; i < eventNumber; i++)
        {
          struct epoll_event triggerEvent = resultEvent[i];
          FDEvent *fdEvent = static_cast<FDEvent *>(triggerEvent.data.ptr);

          // 如果返回的FDEvent是空指针，就没办法处理
          if (fdEvent == nullptr)
          {
            ERRORLOG("fd event = nullptr, continue");
            continue;
          }

          // 触发了读事件
          if (triggerEvent.events & EPOLLIN)
          {
            // 将对应的事件处理器添加到任务队列，让工作线程处理
            DEBUGLOG("fd[%d] trigger EPOLLIN event", fdEvent->GetFD());
            AddTaskToQueue(fdEvent->Handler(FDEvent::IN_EVENT));
          }

          // 触发了写事件
          if (triggerEvent.events & EPOLLOUT)
          {
            // 将对应的事件处理器添加到任务队列，让工作线程处理
            DEBUGLOG("fd[%d] trigger EPOLLOUT event", fdEvent->GetFD());
            AddTaskToQueue(fdEvent->Handler(FDEvent::OUT_EVENT));
          }

          // 触发了错误事件
          if (triggerEvent.events & EPOLLERR)
          {
            DEBUGLOG("fd[%d] trigger EPOLLERR event", fdEvent->GetFD());
            // 删除出错的fd，因为EPOLLERR和EPOLLHUP等事件在epoll上是默认监听的，如果只是取消不删除fd，实际上还是会触发
            DeleteEpollEvent(fdEvent);
            // 执行错误事件处理器
            if (fdEvent->Handler(FDEvent::ERROR_EVENT) != nullptr)
            {
              DEBUGLOG("fd[%d] add error callback to queue", fdEvent->GetFD());
              AddTaskToQueue(fdEvent->Handler(FDEvent::ERROR_EVENT));
            }
          }
        }
      }
    }
  }

  // 关闭服务器的运行
  void EventLoop::Stop()
  {
    myStopFlag = true;

    // 立刻唤醒线程以结束运行
    Wakeup();
  }

  // 处理唤醒线程事件
  void EventLoop::DealWakeUp()
  {
  }

  // 判断当前线程是不是eventloop线程
  bool EventLoop::IsEventLoopThread()
  {
    return myThreadID == GetThreadId();
  }

  // 添加事件到epoll里
  void EventLoop::AddEpollEvent(FDEvent *fdEvent)
  {
    // 只有处于eventloop的线程可以直接添加事件
    if (IsEventLoopThread())
    {
      ADD_TO_EPOLL();
    }
    // 如果是跨线程调用添加事件函数的话，直接把回调函数加入到任务队列里，在eventloop的loop函数中去调用
    else
    {
      // lambda表达式，捕获了this指针和fdEvent，以便在函数体内调用
      auto CallBack = [this, fdEvent]()
      {
        ADD_TO_EPOLL();
      };
      AddTaskToQueue(CallBack, true);
    }
  }

  // 将事件从epoll中删除
  void EventLoop::DeleteEpollEvent(FDEvent *fdEvent)
  {
    // 只有处于当前eventloop线程才可以直接删除事件
    if (IsEventLoopThread())
    {
      DELETE_FROM_EPOILL();
    }
    // 如果是跨线程调用添加事件函数的话，直接把回调函数加入到任务队列里，在eventloop的loop函数中去调用
    else
    {
      auto CallBack = [this, fdEvent]()
      {
        DELETE_FROM_EPOILL();
      };
      AddTaskToQueue(CallBack, true);
    }
  }

  void EventLoop::Wakeup()
  {
    INFOLOG("WAKE UP in eventloop.cc");
    myWakeUpFdEvent->WakeUp();
  }

  void EventLoop::AddTaskToQueue(std::function<void()> CallBack, bool isWakeUp /*=false*/)
  {
    // 任务队列是共享资源，必须加锁保证线程安全
    ScopeMutex<pMutex> threadLock(myMutex);
    myPendingTask.push(CallBack);
    threadLock.unlock();

    // 有必要的话立刻唤醒主线程的epoll处理添加事件
    if (isWakeUp)
    {
      Wakeup();
    }
  }

  void EventLoop::InitWakeUpFdEvent()
  {
    myWakeupfd = eventfd(0, EFD_NONBLOCK);
    if (myWakeupfd < 0)
    {
      ERRORLOG("failed to create event loop, eventfd create error, errno=[%d], error info=%s", errno, strerror(errno));
      exit(0);
    }

    INFOLOG("wakeup fd = [%d]", myWakeupfd);

    myWakeUpFdEvent = new WakeUpFdEvent(myWakeupfd);

    myWakeUpFdEvent->Listen(FDEvent::IN_EVENT, [this]()
                            {
      char buf[8];
      while (read(myWakeupfd, buf, 8) != -1 && errno != EAGAIN) {

      }
      DEBUGLOG("read full bytes from wake up fd[%d]", myWakeupfd); });

    // 将唤醒事件添加到epoll监听表里
    AddEpollEvent(myWakeUpFdEvent);
  }

  void EventLoop::InitTimer()
  {
    myTimer = new Timer();
    // 将定时器添加到epoll监听表里
    AddEpollEvent(myTimer);
  }

  void EventLoop::AddTimerEvent(TimerEvent::myTimerEventPtr timerEvent)
  {
    myTimer->AddTimerEvent(timerEvent);
  }

  EventLoop *EventLoop::GetCurrentEventLoop()
  {
    if (currentEventloop)
    {
      return currentEventloop;
    }
    currentEventloop = new EventLoop();
    return currentEventloop;
  }

  bool EventLoop::isLooping()
  {
    return myIsLooping;
  }
} // namespace myRocket
