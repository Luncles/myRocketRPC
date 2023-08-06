/* ************************************************************************
> File Name:     test_client.cc
> Author:        Luncles
> 功能:          测试tcp_connection
> Created Time:  2023年08月06日 星期日 21时43分43秒
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
#include "/home/luncles/myRocketRPC/net/timer.h"
#include "/home/luncles/myRocketRPC/net/io_thread.h"
#include "/home/luncles/myRocketRPC/net/io_thread_group.h"
#include "../net/tcp/tcp_connection.h"

void test_connect()
{
  // 调用connect连接服务器
  // 发送一个字符串
  // 等待返回结果

  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0)
  {
    ERRORLOG("invalid fd[%d]", fd);
    exit(0);
  }

  struct sockaddr_in serverAddress;
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(12355);
  inet_aton("127.0.0.1", &serverAddress.sin_addr);

  int ret = connect(fd, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress));

  DEBUGLOG("connect success");

  std::string msg = "hello, myRocket!";

  ret = write(fd, msg.c_str(), msg.length());

  DEBUGLOG("success write [%d] bytes to server, fd=[%d], msg=[%s]", ret, fd, msg.c_str());

  char buf[128];

  ret = read(fd, buf, 128);

  DEBUGLOG("success read [%d] bytes from server, fd=[%d], msg=[%s]", ret, fd, msg.c_str());
}

int main()
{
  myRocket::Config::SetGlobalConfig("/home/luncles/myRocketRPC/conf/myRocket.xml");
  myRocket::Logger::InitGlobalLogger(1);

  test_connect();
  return 0;
}