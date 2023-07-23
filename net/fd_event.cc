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

namespace myRocket
{
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
  void FDEvent::Listen(FdTriggerEvent event, std::function<void()> CallBack)
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
    myListenEvents.data.ptr = this;
  }

} // namespace myRocket
