/* ************************************************************************
> File Name:     fd_event.cc
> Author:        Luncles
> 功能:          封装了fd与对应的事件
> Created Time:  2023年07月18日 星期二 20时30分33秒
> Description:
 ************************************************************************/

#include "fd_event.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

namespace myRocket
{
  FDEvent::FDEvent()
  {
    // 这里要初始化一下myListenEvents，不然会得到一个很大的数
    memset(&myListenEvents, 0, sizeof(myListenEvents));
  }

  FDEvent::FDEvent(int fd) : myFD(fd)
  {
    // 这里要初始化一下myListenEvents，不然会得到一个很大的数
    memset(&myListenEvents, 0, sizeof(myListenEvents));
  }

  FDEvent::~FDEvent()
  {
  }

  // 返回与事件相对应的回调函数
  std::function<void()> FDEvent::Handler(FdTriggerEvent event)
  {
    if (event == IN_EVENT)
    {
      return myReadCallBack;
    }
    else if (event == OUT_EVENT)
    {
      return myWriteCallBack;
    }
    else if (event == ERROR_EVENT)
    {
      return myErrorCallBack;
    }

    return nullptr;
  }

  // 设置事件类型与其回调函数，进行监听
  void FDEvent::Listen(FdTriggerEvent event, std::function<void()> CallBack, std::function<void()> errorCallBack)
  {
    if (event == IN_EVENT)
    {
      myListenEvents.events |= EPOLLIN;
      myReadCallBack = CallBack;
    }
    else if (event == OUT_EVENT)
    {
      myListenEvents.events |= EPOLLOUT;
      myWriteCallBack = CallBack;
    }

    if (myErrorCallBack == nullptr)
    {
      myErrorCallBack = errorCallBack;
    }
    else
    {
      myErrorCallBack = nullptr;
    }
    myListenEvents.data.ptr = this;
  }

  // 取消监听事件
  void FDEvent::CancelEvent(FdTriggerEvent eventType)
  {
    if (eventType == FdTriggerEvent::IN_EVENT)
    {
      myListenEvents.events &= (~EPOLLIN);
    }
    else
    {
      myListenEvents.events &= (~EPOLLOUT);
    }
  }

  int FDEvent::SetNonBlock(int fd)
  {
    // 获取文件描述符旧的状态标志
    int oldOption = fcntl(fd, F_GETFL);
    // 设置非阻塞标志
    int newOption = oldOption | O_NONBLOCK;
    // 设置非阻塞
    fcntl(fd, F_SETFL, newOption);
    // 返回文件描述符旧的状态标志，以便日后恢复该状态标志
    return oldOption;
  }

} // namespace myRocket
