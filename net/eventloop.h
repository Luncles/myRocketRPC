/* ************************************************************************
> File Name:     eventloop.h
> Author:        Luncles
> 功能：          Reactor头文件
> Created Time:  Sat 15 Jul 2023 10:11:01 PM CST
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_EVENTLOOP_H
#define MYROCKETRPC_NET_EVENTLOOP_H

#include <pthread.h>

#include <functional>
#include <queue>
#include <set>

#include "/home/luncles/myRocketRPC/common/mutex.h"
#include "fd_event.h"
#include "wakeup_fd_event.h"
#include "timer.h"

namespace myRocket
{
  class EventLoop
  {
  public:
    static EventLoop *GetCurrentEventLoop();

  public:
    EventLoop();

    ~EventLoop();

    // 业务逻辑处理主循环
    void Loop();

    // 唤醒IO线程
    void Wakeup();

    // 处理唤醒事件
    void DealWakeUp();

    // 终止服务器，一般不执行
    void Stop();

    // 添加定时器事件
    void AddTimerEvent(TimerEvent::myTimerEventPtr timerEvent);

    // 添加事件
    void AddEpollEvent(FDEvent *fdEvent);
    // 删除事件
    void DeleteEpollEvent(FDEvent *fdEvent);

    // 判断当前线程是不是eventloop线程
    bool IsEventLoopThread();

    // 将跨线程调用的添加事件回调函数添加到任务队列中
    void AddTaskToQueue(std::function<void()> CallBack, bool isWakeUp = false /*=false*/);

  private:
    // 初始化唤醒fd事件:其实唤醒事件是为了唤醒epoll_wait的阻塞，当没有事件发生又需要唤醒epoll_wait时，就往wakeupFD里写一个字符，这样epoll_wait监听到读事件发生，就会返回了
    void InitWakeUpFdEvent();

    // 初始化定时器
    void InitTimer();

  private:
    pid_t myThreadID{0};                             // 线程id
    int myEpollfd{0};                                // epoll句柄
    int myWakeupfd{0};                               // 用来唤醒IO的fd
    WakeUpFdEvent *myWakeUpFdEvent{nullptr};         // 唤醒fd对应的事件
    Timer *myTimer{nullptr};                         // 定时器指针
    bool myStopFlag{false};                          // 关闭服务器的标志
    std::set<int> myListenfds;                       // 正在监听的fd集合
    std::queue<std::function<void()>> myPendingTask; // 待执行的任务队列
    pMutex myMutex;                                  // 任务队列的互斥锁
  };
} // namespace myRocket

#endif
