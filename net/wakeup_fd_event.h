/* ************************************************************************
> File Name:     wakeup_fd_event.h
> Author:        Luncles
> 功能:          封装特殊的唤醒fd
> Created Time:  2023年07月20日 星期四 22时38分57秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_WAKEUP_FD_EVENT_H
#define MYROCKETRPC_NET_WAKEUP_FD_EVENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "fd_event.h"

namespace myRocketRPC
{
  class WakeUpFdEvent : public FDEvent
  {
  public:
    WakeUpFdEvent(int fd) : FDEvent(fd) {}

    ~WakeUpFdEvent() {}

    // 向eventloop发送一个唤醒字符，以从epoll_wait中唤醒线程
    void WakeUp();

  private:
  };
}

#endif
