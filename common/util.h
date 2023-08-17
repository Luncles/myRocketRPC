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
#include <sys/time.h>

namespace myRocketRPC
{
    pid_t GetPid();
    pid_t GetThreadId();

    // 获取当前时间
    int64_t GetNowMS();

    // 将网络字节序转换为主机字节序
    int32_t GetInt32FromNetByte(const char *buf);
} // namespace myRocketRPC

#endif