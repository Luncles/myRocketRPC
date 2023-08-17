/* ************************************************************************
> File Name:     tinypb_protocol.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月09日 星期三 19时47分05秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tinypb_protocol.h"

namespace myRocketRPC
{
  char TinyProtocol::PB_START = 0x02;
  char TinyProtocol::PB_END = 0x03;
}