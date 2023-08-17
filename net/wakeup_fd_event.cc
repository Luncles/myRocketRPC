/* ************************************************************************
> File Name:     wakeup_fd_event.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年07月20日 星期四 22时39分04秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "wakeup_fd_event.h"
#include "myRocketRPC/common/log.h"

namespace myRocketRPC
{

  void WakeUpFdEvent::WakeUp()
  {
    char wakeupBuf[8] = {'a'};
    int result = write(myFD, wakeupBuf, 8);
    if (result != 8)
    {
      ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", myFD);
    }

    DEBUGLOG("success write 8 bytes, fd[%d]", myFD);
  }

}
