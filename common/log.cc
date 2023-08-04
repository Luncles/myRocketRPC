/* ************************************************************************
> File Name:     log.cpp
> Author:        Luncles
> 功能：          日志模块实现文件
> Created Time:  Sun 09 Jul 2023 10:10:20 PM CST
> Description:
 ************************************************************************/
#include <sys/time.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include "log.h"
#include "util.h"
#include "config.h"
#include "mutex.h"

namespace myRocket
{
    // 单例模式
    static Logger *gLogger = nullptr;

    Logger *Logger::GetGlobalLogger()
    {
        return gLogger;
    }

    void Logger::InitGlobalLogger(int type)
    {
        LogLevel globalLogLevel = StringToLogLevel(Config::GetGlobalConfig()->myLogLevel);
        printf("Initial gLogger [%s]\n", LogLevelToString(globalLogLevel).c_str());
        gLogger = new Logger(globalLogLevel);
    }

    void Logger::PushLog(const std::string &msg)
    {
        ScopeMutex<pMutex> scopeMutex(myMutex); // 加锁
        myBuffer.push(msg);
        if (myBuffer.empty())
        {
            printf("push failure\n");
        }
        scopeMutex.unlock(); // 解锁
    }

    void Logger::Log()
    {
        ScopeMutex<pMutex> scopeMutex(myMutex); // 加锁
        std::queue<std::string> tmp;            // 尽量减小锁的粒度
        myBuffer.swap(tmp);
        scopeMutex.unlock(); // 解锁

        while (!tmp.empty())
        {
            std::string msg = tmp.front();
            tmp.pop();

            puts(msg.c_str());
        }
    }

    std::string LogLevelToString(LogLevel loglevel)
    {
        switch (loglevel)
        {
        case DEBUG:
            return "DEBUG";
            break;
        case INFO:
            return "INFO";
            break;
        case ERROR:
            return "ERROR";
            break;
        default:
            return "UNKNOWN";
            break;
        }
    }

    LogLevel StringToLogLevel(const std::string &loglevelStr)
    {
        // 字符串比较不能用switch
        if (loglevelStr == "DEBUG")
        {
            return DEBUG;
        }
        else if (loglevelStr == "INFO")
        {
            return INFO;
        }
        else if (loglevelStr == "ERROR")
        {
            return ERROR;
        }
        else
        {
            return UNKNOWN;
        }
    }

    std::string LogEvent::toString()
    {
        struct timeval nowTime;
        gettimeofday(&nowTime, nullptr);

        struct tm nowTime_t;
        localtime_r(&(nowTime.tv_sec), &nowTime_t);

        char buf[128];
        strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &nowTime_t);
        std::string timeStr(buf);
        int ms = nowTime.tv_usec / 1000;
        timeStr += ("." + std::to_string(ms));

        myPID = GetPid();
        myThreadID = GetThreadId();

        std::stringstream ss;
        ss << "[" << LogLevelToString(myLevel) << "]\t"
           << "[" << timeStr << "]\t"
           << "[" << std::to_string(myPID) << " : " << std::to_string(myThreadID) << "]\t";

        return ss.str();
    }
} // namespace myR
