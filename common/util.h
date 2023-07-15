/* ************************************************************************
> File Name:     util.h
> Author:        Luncles
> 功能：          辅助函数
> Created Time:  Sun 09 Jul 2023 10:24:06 PM CST
> Description:
 ************************************************************************/

#ifndef MYROCKETRPC_COMMON_UTIL_H
#define MYROCKETRPC_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace myRocket
{
    pid_t GetPid();
    pid_t GetThreadId();
} // namespace myRocketRPC

#endif