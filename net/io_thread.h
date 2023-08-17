/* ************************************************************************
> File Name:     io_thread.h
> Author:        Luncles
> 功能:
> Created Time:  2023年07月26日 星期三 22时09分43秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_IO_THREAD_H
#define MYROCKETRPC_IO_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <semaphore.h>

#include <pthread.h>
#include "eventloop.h"

namespace myRocketRPC
{
  class IOThread
  {
  public:
    IOThread();

    ~IOThread();

    EventLoop *GetEventLoop();

    // 主动启动io线程的eventloop
    void Start();

    // 等待io线程退出
    void Join();

  public:
    static void *ThreadMain(void *arg);

  private:
    pid_t myThreadID{-1};            // 线程号
    pthread_t myThread{0};           // 线程句柄
    EventLoop *myEventLoop{nullptr}; // 当前IO线程的eventloop对象指针

    sem_t myInitSemaphore; // 与主线程进行同步的信号量
    sem_t myStartSemaphore;
  };
}

#endif
