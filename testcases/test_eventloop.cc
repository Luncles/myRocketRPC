/* ************************************************************************
> File Name:     test_eventloop.cc
> Author:        Luncles
> 功能:          测试eventloop模块
> Created Time:  2023年07月22日 星期六 22时07分08秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "myRocketRPC/common/config.h"
#include "myRocketRPC/common/log.h"
#include "myRocketRPC/net/eventloop.h"
#include "myRocketRPC/net/fd_event.h"
#include "myRocketRPC/net/timer.h"
#include "myRocketRPC/net/io_thread.h"
#include "myRocketRPC/net/io_thread_group.h"

void TestIOThread()
{
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);

  if (listenfd == -1)
  {
    ERRORLOG("listenfd create fail");
    exit(1);
  }

  struct sockaddr_in listenAddress;
  memset(&listenAddress, 0, sizeof(listenAddress));
  listenAddress.sin_family = AF_INET;
  listenAddress.sin_port = htons(12355);
  inet_aton("127.0.0.1", &listenAddress.sin_addr);

  int ret = bind(listenfd, (struct sockaddr *)&listenAddress, sizeof(listenAddress));
  if (ret != 0)
  {
    ERRORLOG("bind error!");
    exit(1);
  }

  ret = listen(listenfd, 100);
  if (ret != 0)
  {
    ERRORLOG("listen error!");
    exit(1);
  }

  myRocketRPC::FDEvent fdEvent(listenfd);
  // 将监听fd及其对应读事件添加到epoll事件中，这时候其实还没有添加到epoll监听事件表，只有调用AddEpollEvent函数才会完全开始epoll监听
  fdEvent.Listen(myRocketRPC::FDEvent::FdTriggerEvent::IN_EVENT, [listenfd]()
                 {
    struct sockaddr_in clientAddress;
    socklen_t addressLen = sizeof(clientAddress);
    memset(&clientAddress, 0, sizeof(clientAddress));
    int clientfd = accept(listenfd, (struct sockaddr*)&clientAddress, &addressLen);

    DEBUGLOG("success get client fd[%d], client address:[%s:%d]", clientfd, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port)); });

  // 创建timerEvent
  int timerNum = 0;
  myRocketRPC::TimerEvent::myTimerEventPtr timerEvent = std::make_shared<myRocketRPC::TimerEvent>(
      5000, true, [&timerNum]()
      { INFOLOG("trigger timer event, count=[%d]", timerNum++); });

  // // 这时候才创建出epoll监听
  // myRocket::IOThread ioThread;

  // // 这里相当于在io线程里监听socket了
  // ioThread.GetEventLoop()->AddEpollEvent(&fdEvent);
  // ioThread.GetEventLoop()->AddTimerEvent(timerEvent);

  // // 启动io线程的eventloop，io线程开始工作
  // ioThread.Start();

  // // 要等到io线程退出才能退出主线程
  // ioThread.Join();

  // 测试io线程组
  myRocketRPC::IOThreadGroup ioThreadGroup(2);

  myRocketRPC::IOThread *ioThread1 = ioThreadGroup.GetIOThread();
  ioThread1->GetEventLoop()->AddEpollEvent(&fdEvent);
  ioThread1->GetEventLoop()->AddTimerEvent(timerEvent);

  myRocketRPC::IOThread *ioThread2 = ioThreadGroup.GetIOThread();
  ioThread2->GetEventLoop()->AddTimerEvent(timerEvent);

  ioThreadGroup.Start();

  ioThreadGroup.Join();
}

int main()
{
  myRocketRPC::Config::SetGlobalConfig("/home/luncles/myRocketRPC/conf/myRocket.xml"); // 获取配置参数

  myRocketRPC::Logger::InitGlobalLogger(); // 初始化日志器才能够进行日志打印

  // 到io线程去执行任务
  TestIOThread();

  // myRocket::EventLoop *testEventLoop = new myRocket::EventLoop();

  // int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  // if (listenfd == -1)
  // {
  //   ERRORLOG("listenfd create fail");
  //   exit(1);
  // }

  // struct sockaddr_in listenAddress;
  // memset(&listenAddress, 0, sizeof(listenAddress));
  // listenAddress.sin_family = AF_INET;
  // listenAddress.sin_port = htons(12355);
  // inet_aton("127.0.0.1", &listenAddress.sin_addr);

  // int ret = bind(listenfd, (struct sockaddr *)&listenAddress, sizeof(listenAddress));
  // if (ret != 0)
  // {
  //   ERRORLOG("bind error!");
  //   exit(1);
  // }

  // ret = listen(listenfd, 100);
  // if (ret != 0)
  // {
  //   ERRORLOG("listen error!");
  //   exit(1);
  // }

  // myRocket::FDEvent fdEvent(listenfd);
  // // 将监听fd及其对应读事件添加到epoll监听事件表中
  // fdEvent.Listen(myRocket::FDEvent::FdTriggerEvent::IN_EVENT, [listenfd]()
  //                {
  //   struct sockaddr_in clientAddress;
  //   socklen_t addressLen = sizeof(clientAddress);
  //   memset(&clientAddress, 0, sizeof(clientAddress));
  //   int clientfd = accept(listenfd, (struct sockaddr*)&clientAddress, &addressLen);

  //   DEBUGLOG("success get client fd[%d], client address:[%s:%d]", clientfd, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port)); });
  // testEventLoop->AddEpollEvent(&fdEvent);

  // // 创建timerEvent
  // int timerNum = 0;
  // myRocket::TimerEvent::myTimerEventPtr timerEvent = std::make_shared<myRocket::TimerEvent>(
  //     5000, true, [&timerNum]()
  //     { INFOLOG("trigger timer event, count=[%d]", timerNum++); });
  // testEventLoop->AddTimerEvent(timerEvent);

  // // 开始运行服务器
  // testEventLoop->Loop();

  return 0;
}
