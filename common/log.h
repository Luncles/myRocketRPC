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
#include <semaphore.h>
#include "mutex.h"
#include "../net/timer_event.h"

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

#define ASYNC_LOG_QUEUE_SIZE 1000

    // 一些转换函数
    std::string LogLevelToString(LogLevel);
    LogLevel StringToLogLevel(const std::string &loglevelStr);

#define DEBUGLOG(str, ...)                                                                                                                                                                                                                   \
    if (myRocket::Logger::GetGlobalLogger()->GetMySetLevel() <= myRocket::DEBUG)                                                                                                                                                             \
    {                                                                                                                                                                                                                                        \
        myRocket::Logger::GetGlobalLogger()->PushLog(myRocket::LogEvent(myRocket::LogLevel::DEBUG).toString() + "[" + std::string(__FILE__) + " : " + std::to_string(__LINE__) + "]\t" + myRocket::FormatString(str, ##__VA_ARGS__) + "\n"); \
    }

#define INFOLOG(str, ...)                                                                                                                                                                                                                   \
    if (myRocket::Logger::GetGlobalLogger()->GetMySetLevel() <= myRocket::INFO)                                                                                                                                                             \
    {                                                                                                                                                                                                                                       \
        myRocket::Logger::GetGlobalLogger()->PushLog(myRocket::LogEvent(myRocket::LogLevel::INFO).toString() + "[" + std::string(__FILE__) + " : " + std::to_string(__LINE__) + "]\t" + myRocket::FormatString(str, ##__VA_ARGS__) + "\n"); \
    }

#define ERRORLOG(str, ...)                                                                                                                                                                                                                   \
    if (myRocket::Logger::GetGlobalLogger()->GetMySetLevel() <= myRocket::ERROR)                                                                                                                                                             \
    {                                                                                                                                                                                                                                        \
        myRocket::Logger::GetGlobalLogger()->PushLog(myRocket::LogEvent(myRocket::LogLevel::ERROR).toString() + "[" + std::string(__FILE__) + " : " + std::to_string(__LINE__) + "]\t" + myRocket::FormatString(str, ##__VA_ARGS__) + "\n"); \
    }

// 打印应用日志的方法只在rpc服务里调用
#define APPDEBUGLOG(str, ...)                                                                                                                                                                                                                   \
    if (myRocket::Logger::GetGlobalLogger()->GetMySetLevel() <= myRocket::DEBUG)                                                                                                                                                                \
    {                                                                                                                                                                                                                                           \
        myRocket::Logger::GetGlobalLogger()->PushAppLog(myRocket::LogEvent(myRocket::LogLevel::DEBUG).toString() + "[" + std::string(__FILE__) + " : " + std::to_string(__LINE__) + "]\t" + myRocket::FormatString(str, ##__VA_ARGS__) + "\n"); \
    }

#define APPINFOLOG(str, ...)                                                                                                                                                                                                                   \
    if (myRocket::Logger::GetGlobalLogger()->GetMySetLevel() <= myRocket::INFO)                                                                                                                                                                \
    {                                                                                                                                                                                                                                          \
        myRocket::Logger::GetGlobalLogger()->PushAppLog(myRocket::LogEvent(myRocket::LogLevel::INFO).toString() + "[" + std::string(__FILE__) + " : " + std::to_string(__LINE__) + "]\t" + myRocket::FormatString(str, ##__VA_ARGS__) + "\n"); \
    }

#define APPERRORLOG(str, ...)                                                                                                                                                                                                                   \
    if (myRocket::Logger::GetGlobalLogger()->GetMySetLevel() <= myRocket::ERROR)                                                                                                                                                                \
    {                                                                                                                                                                                                                                           \
        myRocket::Logger::GetGlobalLogger()->PushAppLog(myRocket::LogEvent(myRocket::LogLevel::ERROR).toString() + "[" + std::string(__FILE__) + " : " + std::to_string(__LINE__) + "]\t" + myRocket::FormatString(str, ##__VA_ARGS__) + "\n"); \
    }

    // 异步日志类
    class AsyncLogger
    {
    public:
        static void *Loop(void *);

    public:
        // 消费者线程号
        pthread_t myThread;

    public:
        using myAsyncLoggerPtr = std::shared_ptr<AsyncLogger>;

        AsyncLogger(const std::string &fileName, const std::string &filePath, int maxSize);

        void Stop();

        // 将日志刷新到磁盘
        void FlushLogToDisk();

        // 将日志存到异步日志器的缓冲区
        void PushLogToAsyncBuffer(std::vector<std::string> &vec);

    private:
        // 日志文件格式：myFilePath/myFileName_yyyymmdd.myFileNo

        // 缓冲区是一个队列，防止读线程在打印线程还没打印完缓冲区的数据时直接写入，将未打印的日志覆盖
        std::queue<std::vector<std::string>> myAsyncLogBuffer; // 共享内存，注意线程安全

        // 日志输出文件名字
        std::string myFileName;

        // 日志输出路径
        std::string myFilePath;

                // 单个日志文件的最大大小，单位为字节
        int myMaxFileSize{0};

        // 生产者消费者模型，有两个信号量进行资源的同步
        sem_t fullSem;
        sem_t emptySem;

        // 这个信号量是用来控制打印线程（消费线程）完成初始步骤的
        sem_t loggerInitSem;

        // 条件变量，用来通知等待资源的线程
        pthread_cond_t myCondition;

        // 互斥锁
        pMutex myMutex;

        // 当前打印文件的文件日期
        std::string myDate;

        // 当前打开的日志文件句柄，避免经常要打开关闭句柄，影响性能
        FILE *my_file_handler{nullptr};

        // 是否要重新打开日志：当文件重命名，日志滚动（当前日志打满，需要打入到下一个文件中），过了凌晨12点时需要重新打开日志文件
        bool myReopenFlag{false};

        // 当前的日志文件序号
        int myFileNo{0};

        // 停止标志
        bool myStopFlag{false};
    };

    /*日志器*/
    class Logger
    {
    public:
        static Logger *GetGlobalLogger(); // 获取全局日志器

        static void InitGlobalLogger(int type = 1); // 初始化日志器函数

    public:
        using myLoggerPtr = std::shared_ptr<Logger>;

        // 构造函数
        Logger(LogLevel loglevel, int type = 1);

        LogLevel GetLogLevel() const
        {
            return mySetLevel;
        }

        // 将一条底层日志添加到局部日志器Logger的buffer中
        void PushLog(const std::string &msg);

        // 将一条应用日志添加到局部日志器Logger的app buffer中
        void PushAppLog(const std::string &msg);

        // 初始化一个日志器，主要是启动定时器
        void InitLogger();

        // 该函数将每一条日志打印到终端，在项目开发过程中使用
        void Log();

        LogLevel GetMySetLevel()
        {
            return mySetLevel;
        }

        // 每隔一段时间将日志同步到异步日志器里
        void SyncLogLoop();

    private:
        LogLevel mySetLevel;
        std::vector<std::string> myBuffer; // 共享内存，注意线程安全

        std::vector<std::string> myAppBuffer; // 保存应用日志的缓冲区，共享内存，注意线程安全

        pMutex myMutex; // 互斥锁
        pMutex myAppMutex;

        std::string myFileName; // 日志输出文件名字
        std::string myFilePath; // 日志输出路径
        int myMaxFileSize{0};   // 日志单个文件最大字节数

        // 异步日志器
        AsyncLogger::myAsyncLoggerPtr myAsynLogger;

        AsyncLogger::myAsyncLoggerPtr myAsynAppLogger;

        // 定时器
        TimerEvent::myTimerEventPtr myTimerEvent;

        int myType{0};
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
