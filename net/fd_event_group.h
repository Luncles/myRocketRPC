/* ************************************************************************
> File Name:     fd_event_group.h
> Author:        Luncles
> 功能:          fd_event的封装，相当于一个套接字池
> Created Time:  2023年08月04日 星期五 21时38分42秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_FD_EVENT_GROUP_H
#define MYROCKETRPC_NET_FD_EVENT_GROUP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>
#include "fd_event.h"
#include "/home/luncles/myRocketRPC/common/mutex.h"

namespace myRocket
{
  class FDEventGroup
  {
  public:
    static FDEventGroup *GetFDEventGroup();

  public:
    FDEventGroup(int size);

    ~FDEventGroup();

    FDEvent *GetFDEvent(int fd);

  private:
    std::vector<FDEvent *> myFDEventGroup;
    int mySize{0};
    pMutex myMutex;
  };
}

#endif