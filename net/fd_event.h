/* ************************************************************************
> File Name:     fd_event.h
> Author:        Luncles
> 功能:          封装了fd与对应的事件
> Created Time:  2023年07月18日 星期二 20时30分33秒
> Description:
 ************************************************************************/

#ifndef MYROCKETRPC_NET_FD_EVENT_H
#define MYROCKETRPC_NET_FD_EVENT_H

#include <sys/epoll.h>

#include <functional>

namespace myRocketRPC
{
  class FDEvent
  {
  public:
    // 触发的事件类型
    enum FdTriggerEvent
    {
      IN_EVENT = EPOLLIN,
      OUT_EVENT = EPOLLOUT,
      ERROR_EVENT = EPOLLERR,
    };

    FDEvent();

    FDEvent(int fd);

    ~FDEvent();

    // 将fd设置为非阻塞
    int SetNonBlock(int fd);

    // 添加监听事件
    void Listen(FdTriggerEvent eventType, std::function<void()> CallBack, std::function<void()> errorCallBack = nullptr);

    // 取消监听事件
    void CancelEvent(FdTriggerEvent eventType);

    // 获取事件类型对应的回调函数
    std::function<void()> Handler(FdTriggerEvent eventType);

    // 获取对应的fd
    int GetFD() const { return myFD; }

    // 获取对应的epoll事件
    struct epoll_event GetEpollEvent() { return myListenEvents; }

  protected:
    int myFD{-1};

    struct epoll_event myListenEvents;

    std::function<void()> myReadCallBack{nullptr};
    std::function<void()> myWriteCallBack{nullptr};
    std::function<void()> myErrorCallBack{nullptr};
  };
} // namespace myRocket
#endif