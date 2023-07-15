/* ************************************************************************
> File Name:     log.h
> Author:        Luncles
> 功能：         RPC日志模块
> Created Time:  Thu 06 Jul 2023 10:45:11 PM CST
> Description:
                1. 日志级别
                2. 打印到文件，支持日期命名，以及日志的滚动。
                3. c 格式化风控
                4. 线程安全

                LogLevel:
                Debug
                Info
                Error

                LogEvent:
                文件名、行号
                MsgNo
                进程号
                Thread id
                日期，以及时间。精确到 ms
                自定义消息
                日志格式

                [Level][%y-%m-%d %H:%M:%s.%ms]\t[pid:thread_id]\t[file_name:line][%msg]
                Logger 日志器 1.提供打印日志的方法 2.设置日志输出的路径
 ************************************************************************/
#ifndef MYROCKETRPC_COMMON_LOG_H
#define MYROCKETRPC_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include "mutex.h"

namespace myRocket
{
    /*C风格化输出*/
    template <typename... Args>
    std::string FormatString(const char *str, Args &&...args)
    {
        int size = snprintf(nullptr, 0, str, args...);

        std::string results;
        if (size > 0)
        {
            results.resize(size);
            snprintf(&results[0], size + 1, str, args...); // size + 1包括'\0'
        }
        return results;
    }

    enum LogLevel
    {
        UNKNOWN = 0,
        DEBUG = 1,
        INFO = 2,
        ERROR = 3
    };

    // 一些转换函数
    std::string LogLevelToString(LogLevel);
    LogLevel StringToLogLevel(const std::string& loglevelStr);

#define DEBUGLOG(str, ...)                                                                                                           \
    if (myRocket::Logger::GetGlobalLogger()->GetMySetLevel() <= myRocket::DEBUG) \
    { \
        myRocket::Logger::GetGlobalLogger()->PushLog(myRocket::LogEvent(myRocket::LogLevel::DEBUG).toString() + myRocket::FormatString(str, ##__VA_ARGS__) + "\n");   \
        myRocket::Logger::GetGlobalLogger()->Log(); \
    } \
    
#define INFOLOG(str, ...)                                                                                                            \
    if (myRocket::Logger::GetGlobalLogger()->GetMySetLevel() <= myRocket::INFO) \
    { \
        myRocket::Logger::GetGlobalLogger()->PushLog(myRocket::LogEvent(myRocket::LogLevel::INFO).toString() + myRocket::FormatString(str, ##__VA_ARGS__) + "\n");    \
        myRocket::Logger::GetGlobalLogger()->Log(); \
    } \
    
#define ERRORLOG(str, ...)                                                       \
    if (myRocket::Logger::GetGlobalLogger()->GetMySetLevel() <= myRocket::ERROR ) \
    { \
        myRocket::Logger::GetGlobalLogger()->PushLog(myRocket::LogEvent(myRocket::LogLevel::ERROR).toString() + myRocket::FormatString(str, ##__VA_ARGS__) + "\n");   \
        myRocket::Logger::GetGlobalLogger()->Log(); \
    } \
   
    /*日志器*/
    class Logger
    {
    public:
        static Logger *GetGlobalLogger(); // 获取全局日志器

        static void InitGlobalLogger(int type = 1); // 初始化日志器函数


    public:
        using sptrLoger = std::shared_ptr<Logger>;

        // 构造函数
        Logger(LogLevel loglevel) : mySetLevel(loglevel) { }

        LogLevel GetLogLevel() const
        {
            return mySetLevel;
        }

        void PushLog(const std::string &msg); // 日志入队列

        void InitLogger();

        void Log(); // 打印日志

        LogLevel GetMySetLevel() {
            return mySetLevel;
        }

        

    private:
        LogLevel mySetLevel;
        std::queue<std::string> myBuffer;       // 共享内存，注意线程安全
        pMutex myMutex;          // 互斥锁

        std::string myFileName; // 日志输出文件名字
        std::string myFilePath; // 日志输出路径
        int myMaxFileSize{0};   // 日志单个文件最大字节数
    };

    class LogEvent
    {
    public:
        LogEvent(LogLevel level) : myLevel(level) {} // 构造函数

        std::string GetFileName() // 获取文件名
        {
            return myFileName;
        }

        LogLevel GetLogLevel() // 获取日志级别
        {
            return myLevel;
        }

        // 一些转换函数
        std::string toString();

    private:
        std::string myFileName;
        int32_t myFileLine;
        int32_t myPID;
        int32_t myThreadID;

        LogLevel myLevel;
    };
}

#endif
