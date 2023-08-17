/* ************************************************************************
> File Name:     fd_event_group.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月04日 星期五 21时38分47秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "fd_event_group.h"
#include "myRocketRPC/common/log.h"

namespace myRocketRPC
{
  static FDEventGroup *fdEventGroupPtr = nullptr;
  FDEventGroup *FDEventGroup::GetFDEventGroup()
  {
    if (fdEventGroupPtr)
    {
      return fdEventGroupPtr;
    }

    fdEventGroupPtr = new FDEventGroup(128);
    return fdEventGroupPtr;
  }

  FDEventGroup::FDEventGroup(int size) : mySize(size)
  {
    for (int i = 0; i < mySize; i++)
    {
      myFDEventGroup.push_back(new FDEvent(i));
    }
  }

  FDEventGroup::~FDEventGroup()
  {
    for (int i = 0; i < mySize; i++)
    {
      if (myFDEventGroup[i] != nullptr)
      {
        delete myFDEventGroup[i];
        myFDEventGroup[i] = nullptr;
      }
    }
  }

  FDEvent *FDEventGroup::GetFDEvent(int fd)
  {
    // 因为是所有线程都可以申请fd_event，所以需要加锁
    ScopeMutex<pMutex> lock(myMutex);

    DEBUGLOG("Now get a fdevent from fdevent group, fd=[%d]", fd);
    // 先检查当前的fd_event池是否有要申请的fd
    if (fd < mySize)
    {
      return myFDEventGroup[fd];
    }

    // 如果没有，就要扩容
    int newSize = 2 * fd;
    for (int i = mySize; i < newSize; i++)
    {
      myFDEventGroup.push_back(new FDEvent(i));
    }
    mySize = newSize;
    // 先在这里解锁吧，这里不解锁也行，因为RAII，函数运行完后自动就释放了
    lock.unlock();
    return myFDEventGroup[fd];
  }
}