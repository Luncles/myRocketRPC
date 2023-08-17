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
#include <assert.h>
#include <signal.h>
#include "log.h"
#include "util.h"
#include "config.h"
#include "mutex.h"
#include "run_time.h"
#include "myRocketRPC/net/eventloop.h"

namespace myRocketRPC
{
    // 单例模式
    static Logger *gLogger = nullptr;

    // 日志宕机处理函数
    void CoredumpHandler(int signal_no)
    {
        ERRORLOG("process received invalid signal, will exit");
        gLogger->FlushLogToDisk();

        // 等待两个打印线程退出
        pthread_join(gLogger->GetAsyncLogger()->myThread, nullptr);
        pthread_join(gLogger->GetAsyncAppLogger()->myThread, nullptr);

        // 将信号signal_no的处理程序重置为默认值
        signal(signal_no, SIG_DFL);
        // 重新发送信号，一般终止程序
        raise(signal_no);
    }

    Logger *Logger::GetGlobalLogger()
    {
        return gLogger;
    }

    Logger::Logger(LogLevel loglevel, int type /*=1*/) : mySetLevel(loglevel), myType(type)
    {
        // 如果myType为0，则是客户端日志，用不上日志器
        if (myType == 0)
        {
            return;
        }

        myAsynLogger = std::make_shared<AsyncLogger>(
            Config::GetGlobalConfig()->myLogFileName + "_rpc",
            Config::GetGlobalConfig()->myLogFilePath,
            Config::GetGlobalConfig()->myMaxFileSize);

        myAsynAppLogger = std::make_shared<AsyncLogger>(
            Config::GetGlobalConfig()->myLogFileName + "_app",
            Config::GetGlobalConfig()->myAppLogFilePath,
            Config::GetGlobalConfig()->myMaxFileSize);
    }

    // 每隔一段时间将日志同步到异步日志器里
    void Logger::SyncLogLoop()
    {
        // 同步myBuffer到AsyncLogger的myAsyncLogBuffer队尾
        // 减少锁的粒度
        std::vector<std::string> tmpVec;
        ScopeMutex<pMutex> lock(myMutex);
        tmpVec.swap(myBuffer);
        lock.unlock();

        if (!tmpVec.empty())
        {
            myAsynLogger->PushLogToAsyncBuffer(tmpVec);
        }
        tmpVec.clear();

        // 同步myAppBuffer到AsyncLogger的myAsyncLogBuffer队尾
        // 减少锁的粒度
        std::vector<std::string> tmpAppVec;
        ScopeMutex<pMutex> lock2(myAppMutex);
        tmpAppVec.swap(myAppBuffer);
        lock2.unlock();

        if (!tmpAppVec.empty())
        {
            myAsynAppLogger->PushLogToAsyncBuffer(tmpAppVec);
        }
        tmpAppVec.clear();
    }

    void Logger::InitLogger()
    {
        if (myType == 0)
        {
            return;
        }
        // 在使用 std::bind 时，即使您的 SyncLogLoop 函数是定义在同一个类中，也需要加上作用域限定符，因为 std::bind 是一个模板函数，它不知道您要绑定的函数是哪个类的成员函数。因此，您需要使用作用域限定符来指定要绑定的函数所属的类。
        myTimerEvent = std::make_shared<TimerEvent>(Config::GetGlobalConfig()->myLogSyncInterval, true, std::bind(&myRocketRPC::Logger::SyncLogLoop, this));

        // 将同步日志定时器添加到eventloop里
        EventLoop::GetCurrentEventLoop()->AddTimerEvent(myTimerEvent);

        // 添加异常处理，使程序更健壮
        signal(SIGSEGV, CoredumpHandler);
        signal(SIGABRT, CoredumpHandler);
        signal(SIGTERM, CoredumpHandler);
        signal(SIGKILL, CoredumpHandler);
        signal(SIGINT, CoredumpHandler);
        signal(SIGSTKFLT, CoredumpHandler);
    }

    void Logger::InitGlobalLogger(int type)
    {
        LogLevel globalLogLevel = StringToLogLevel(Config::GetGlobalConfig()->myLogLevel);
        printf("Initial gLogger [%s]\n", LogLevelToString(globalLogLevel).c_str());
        gLogger = new Logger(globalLogLevel, type);

        // 启动定时器
        gLogger->InitLogger();
    }

    void Logger::PushLog(const std::string &msg)
    {
        // 如果是客户端的是，直接打印到控制台，不用记录日志文件了
        if (myType == 0)
        {
            printf("%s\n", msg.c_str());
        }
        ScopeMutex<pMutex> scopeMutex(myMutex); // 加锁
        myBuffer.push_back(msg);
        if (myBuffer.empty())
        {
            printf("push rpc log failure\n");
        }
        scopeMutex.unlock(); // 解锁
    }

    void Logger::PushAppLog(const std::string &msg)
    {
        ScopeMutex<pMutex> scopeAppMutex(myMutex); // 加锁
        myAppBuffer.push_back(msg);
        if (myAppBuffer.empty())
        {
            printf("push app log failure\n");
        }
        scopeAppMutex.unlock(); // 解锁
    }

    void Logger::Log()
    {
        ScopeMutex<pMutex> scopeMutex(myMutex); // 加锁
        std::vector<std::string> tmp;           // 尽量减小锁的粒度
        myBuffer.swap(tmp);
        scopeMutex.unlock(); // 解锁

        while (!tmp.empty())
        {
            std::string msg = tmp.front();
            tmp.erase(tmp.begin());

            puts(msg.c_str());
        }
    }

    void Logger::FlushLogToDisk()
    {
        // 先把当前的局部缓冲区的日志刷新到打印线程的缓冲区里
        SyncLogLoop();
        myAsynLogger->Stop();
        myAsynLogger->FlushLogToDisk();

        myAsynAppLogger->Stop();
        myAsynAppLogger->FlushLogToDisk();
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

        // 获取当前线程处理的请求message id和methd
        std::string messageID = RunTime::GetRunTime()->messageID;
        std::string method = RunTime::GetRunTime()->methodName;
        if (!messageID.empty())
        {
            ss << "[" << messageID << "]\t";
        }
        if (!method.empty())
        {
            ss << "[" << method << "]\t";
        }
        return ss.str();
    }

    void *AsyncLogger::Loop(void *arg)
    {
        // 消费者线程的循环，将buffer里的全部数据打印到文件中，然后线程睡眠，直到有新的数据再重复这个过程
        AsyncLogger *logger = reinterpret_cast<AsyncLogger *>(arg);

        // 用条件变量来唤醒等待打印的消费者线程
        assert(pthread_cond_init(&logger->myCondition, nullptr) == 0);

        // 起码到这里才算完成异步日志器的定义
        sem_post(&logger->loggerInitSem);

        while (1)
        {
            sem_wait(&logger->fullSem);
            ScopeMutex<pMutex> lock(logger->myMutex);
            // 这里用while而不用if是为了避免虚假唤醒的问题，虚假唤醒是指线程在没有收到条件变量的信号时被唤醒。
            // 使用while时，当线程被虚假唤醒时，会去检查logger->myAsyncLogBuffer.empty()是否为真
            // 如果还是空的，就会继续进入等待。
            // 如果用的是if,那么被虚假唤醒的线程就会直接往下执行了，这时候就错了
            while (logger->myAsyncLogBuffer.empty())
            {
                pthread_cond_wait(&(logger->myCondition), logger->myMutex.GetMutex());
            }

            std::vector<std::string> tmp;
            tmp.swap(logger->myAsyncLogBuffer.front());
            logger->myAsyncLogBuffer.pop();
            lock.unlock();
            sem_post(&logger->emptySem);

            // 接下来获取当前时间
            timeval now;
            gettimeofday(&now, nullptr);

            struct tm nowTime;
            // 将当前时间（秒）转换成本地时间，有年、月、日、时、分、秒等时间信息
            localtime_r(&(now.tv_sec), &nowTime);

            // 日期格式
            const char *format = "%Y%m%d";
            char date[32];
            // 格式化日期
            strftime(date, sizeof(date), format, &nowTime);

            // 如果已经过了凌晨12点
            if (std::string(date) != logger->myDate)
            {
                logger->myFileNo = 0; // 跨了天，命名的序号要重新开始
                logger->myReopenFlag = true;
                logger->myDate = std::string(date); // 日期也要改变了
            }

            // 打开一个之前没有打开过的新文件
            if (logger->my_file_handler == nullptr)
            {
                logger->myReopenFlag = true;
            }

            std::stringstream ss;
            ss << logger->myFilePath << logger->myFileName << "_" << std::string(date) << "_log.";
            std::string logFileName = ss.str() + std::to_string(logger->myFileNo);

            // 如果需要重新打开文件
            if (logger->myReopenFlag)
            {
                // 如果之前已经打开了一个文件，就把之前的文件关闭
                if (logger->my_file_handler)
                {
                    fclose(logger->my_file_handler);
                }
                // 以追加末尾的方式打开新文件
                logger->my_file_handler = fopen(logFileName.c_str(), "a");
                if (logger->my_file_handler == nullptr)
                {
                    perror("Error");
                    return nullptr;
                }
                logger->myReopenFlag = false;
            }

            // 如果当前文件的写入字节数已经超限了，则关闭日志，另开一个
            // ftell：返回当前字节流的位置
            if (ftell(logger->my_file_handler) > logger->myMaxFileSize)
            {
                fclose(logger->my_file_handler);

                logFileName = ss.str() + std::to_string(logger->myFileNo++);
                logger->my_file_handler = fopen(logFileName.c_str(), "a");
                logger->myReopenFlag = false;
            }

            for (auto &i : tmp)
            {
                if (!i.empty())
                {
                    fwrite(i.c_str(), i.length(), 1, logger->my_file_handler);
                }
            }

            // 刷盘
            fflush(logger->my_file_handler);

            if (logger->myStopFlag)
            {
                return nullptr;
            }
        }
        return nullptr;
    }

    AsyncLogger::AsyncLogger(const std::string &fileName, const std::string &filePath, int maxSize) : myFileName(fileName), myFilePath(filePath), myMaxFileSize(maxSize)
    {
        // 先初始化两个信号量
        sem_init(&fullSem, 0, 0);
        sem_init(&emptySem, 0, ASYNC_LOG_QUEUE_SIZE);
        sem_init(&loggerInitSem, 0, 0);

        assert(pthread_create(&myThread, nullptr, &Loop, this) == 0);

        // 要等打印线程初始化完条件变量才能算完成异步日志器
        sem_wait(&loggerInitSem);
    }

    void AsyncLogger::Stop()
    {
        // 设置为true，那么就会退出异步日志的循环了
        myStopFlag = true;
    }

    // 将日志刷新到磁盘
    void AsyncLogger::FlushLogToDisk()
    {
        if (my_file_handler)
        {
            fflush(my_file_handler);
        }
    }

    // 将日志存到缓存区
    void AsyncLogger::PushLogToAsyncBuffer(std::vector<std::string> &vec)
    {
        sem_wait(&emptySem);
        ScopeMutex<pMutex> lock(myMutex);
        myAsyncLogBuffer.push(vec);

        // 有数据了，用条件变量唤醒沉睡中的异步日志线程
        pthread_cond_signal(&myCondition);
        lock.unlock();
        sem_post(&fullSem);
    }

} // namespace myR
