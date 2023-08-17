/* ************************************************************************
> File Name:     io_thread_group.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月02日 星期三 21时23分31秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "io_thread_group.h"

namespace myRocketRPC
{
  IOThreadGroup::IOThreadGroup(size_t size) : mySize(size)
  {
    myIOThreadGroups.resize(mySize);
    for (size_t i = 0; i < myIOThreadGroups.size(); i++)
    {
      myIOThreadGroups[i] = new IOThread();
    }
  }

  IOThreadGroup::~IOThreadGroup()
  {
  }

  void IOThreadGroup::Start()
  {
    for (size_t i = 0; i < myIOThreadGroups.size(); i++)
    {
      myIOThreadGroups[i]->Start();
    }
  }

  void IOThreadGroup::Join()
  {
    for (size_t i = 0; i < myIOThreadGroups.size(); i++)
    {
      myIOThreadGroups[i]->Join();
    }
  }

  IOThread *IOThreadGroup::GetIOThread()
  {
    size_t temp = myIndex;
    myIndex = (myIndex + 1) % myIOThreadGroups.size();
    return myIOThreadGroups[temp];
  }
}
