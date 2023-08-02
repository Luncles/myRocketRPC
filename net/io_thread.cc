/* ************************************************************************
> File Name:     io_thread.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年07月26日 星期三 22时09分48秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sstream>

#include "/home/luncles/myRocketRPC/common/log.h"
#include "/home/luncles/myRocketRPC/common/util.h"
#include "io_thread.h"

namespace myRocket
{
  IOThread::IOThread()
  {

    // 创建一个信号量来同步主线程和io线程，初始值设为0，在同一进程下运行，因此pshared也为0
    int ret = sem_init(&myInitSemaphore, 0, 0);
    if (ret != 0)
    {
      ERRORLOG("init InitSemaphore failed");
      exit(-1);
    }

    // 创建一个信号量用来主动启动io线程的eventloop
    ret = sem_init(&myStartSemaphore, 0, 0);
    if (ret != 0)
    {
      ERRORLOG("init StartSemaphore failed");
      exit(-1);
    }

    // 创建io线程执行业务逻辑
    pthread_create(&myThread, nullptr, ThreadMain, this);

    // 等待信号量，直到io线程执行完前置部分
    sem_wait(&myInitSemaphore);

    DEBUGLOG("IOThread[%d] create success", myThreadID);
  }

  IOThread::~IOThread()
  {
    // 退出io线程先要停止eventloop
    myEventLoop->Stop();

    // 销毁信号量
    sem_destroy(&myInitSemaphore);
    sem_destroy(&myStartSemaphore);

    // 等待io线程执行完毕退出
    pthread_join(myThread, nullptr);

    if (myEventLoop)
    {
      delete myEventLoop;
      myEventLoop = nullptr;
    }
  }

  // 静态函数
  void *IOThread::ThreadMain(void *arg)
  {
    // 传入的实参是IOThread指针，而形参是void*，因此需要先进行转换
    IOThread *ioThread = static_cast<IOThread *>(arg);

    // 每个线程有各自的eventloop
    ioThread->myEventLoop = new EventLoop();

    // 获取当前线程号
    ioThread->myThreadID = GetThreadId();

    // 前置的部分执行完毕了，可以唤醒信号量，让主线程继续执行了
    sem_post(&ioThread->myInitSemaphore);

    // 让IO线程等待，直到主动去启动子线程的eventloop
    DEBUGLOG("IOThread [%d] created, wait start semaphore", ioThread->myThreadID);
    sem_wait(&ioThread->myStartSemaphore);

    // 如果执行到这里，说明拿到了信号量，开始启动io线程的eventloop了
    DEBUGLOG("IOThread [%d] start loop", ioThread->myThreadID);
    ioThread->myEventLoop->Loop();

    // 如果eventloop跳出了loop函数，说明结束了
    DEBUGLOG("IOThread [%d] end loop", ioThread->myThreadID);

    return nullptr;
  }

  EventLoop *IOThread::GetEventLoop()
  {
    return myEventLoop;
  }

  void IOThread::Start()
  {
    DEBUGLOG("Now start IOThread [%d]", myThreadID);
    sem_post(&myStartSemaphore);
  }

  void IOThread::Join()
  {
    pthread_join(myThread, nullptr);
  }
}
