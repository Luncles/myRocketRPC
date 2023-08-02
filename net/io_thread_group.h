/* ************************************************************************
> File Name:     io_thread_group.h
> Author:        Luncles
> 功能:          IO线程池
> Created Time:  2023年08月02日 星期三 21时23分26秒
> Description:
 ************************************************************************/
#ifndef MYROCKET_NET_IO_THREAD_GROUP_H
#define MYROCKET_NET_IO_THREAD_GROUP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>
#include "io_thread.h"

namespace myRocket
{
  class IOThreadGroup
  {
  public:
    IOThreadGroup(size_t size);

    ~IOThreadGroup();

    void Start();

    void Join();

    IOThread *GetIOThread();

  private:
    size_t mySize{0};
    std::vector<IOThread *> myIOThreadGroups;
    size_t myIndex{0}; // 指向下个待分配的线程索引
  };
}

#endif
