/* ************************************************************************
> File Name:     util.cpp
> Author:        Luncles
> 功能：
> Created Time:  Sun 09 Jul 2023 10:24:11 PM CST
> Description:
 ************************************************************************/

#include <syscall.h>
#include "util.h"

namespace myRocket
{
    static int gPid = 0;
    static thread_local int gThreadId = 0;

    pid_t GetPid()
    {
        if (gPid != 0)
        {
            return gPid;
        }
        return getpid();
    }

    pid_t GetThreadId()
    {
        if (gThreadId != 0)
        {
            return gThreadId;
        }
        return syscall(SYS_gettid); // 间接系统调用，获取真实的线程id唯一标识
    }
} // namespace myRocketRPC
