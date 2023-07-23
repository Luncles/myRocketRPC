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
#include "/home/luncles/myRocketRPC/common/config.h"
#include "/home/luncles/myRocketRPC/common/log.h"
#include "/home/luncles/myRocketRPC/net/eventloop.h"
#include "/home/luncles/myRocketRPC/net/fd_event.h"

int main()
{
  myRocket::Config::SetGlobalConfig("/home/luncles/myRocketRPC/conf/myRocket.xml"); // 获取配置参数

  myRocket::Logger::InitGlobalLogger(); // 初始化日志器才能够进行日志打印

  myRocket::EventLoop *testEventLoop = new myRocket::EventLoop();

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

  myRocket::FDEvent fdEvent(listenfd);
  // 将监听fd及其对应读事件添加到epoll监听事件表中
  fdEvent.Listen(myRocket::FDEvent::FdTriggerEvent::IN_EVENT, [listenfd]()
                 {
    struct sockaddr_in clientAddress;
    socklen_t addressLen = sizeof(clientAddress);
    memset(&clientAddress, 0, sizeof(clientAddress));
    int clientfd = accept(listenfd, (struct sockaddr*)&clientAddress, &addressLen);

    DEBUGLOG("success get client fd[%d], client address:[%s:%d]", clientfd, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port)); });
  testEventLoop->AddEpollEvent(&fdEvent);

  // 开始运行服务器
  testEventLoop->Loop();

  return 0;
}
