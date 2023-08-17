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
#include <memory>
#include "myRocketRPC/common/config.h"
#include "myRocketRPC/common/log.h"
#include "myRocketRPC/net/eventloop.h"
#include "myRocketRPC/net/fd_event.h"
#include "myRocketRPC/net/timer.h"
#include "myRocketRPC/net/io_thread.h"
#include "myRocketRPC/net/io_thread_group.h"
#include "myRocketRPC/net/tcp/tcp_connection.h"
#include "myRocketRPC/net/tcp/tcp_client.h"
#include "myRocketRPC/net/coder/string_coder.h"
#include "myRocketRPC/net/coder/tinypb_coder.h"
#include "myRocketRPC/net/coder/tinypb_protocol.h"

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

void test_connect_client()
{
  myRocketRPC::IPNetAddr::myNetAddrPtr addrPtr = std::make_shared<myRocketRPC::IPNetAddr>("127.0.0.1", 12355);
  DEBUGLOG("create address [%s]", addrPtr->ToString().c_str());
  myRocketRPC::TcpClient tcpClient(addrPtr);
  tcpClient.ConnectServer([addrPtr]()
                          { DEBUGLOG("success connect to [%s]", addrPtr->ToString().c_str()); });
}

void test_tcp_client()
{
  myRocketRPC::IPNetAddr::myNetAddrPtr addrPtr = std::make_shared<myRocketRPC::IPNetAddr>("127.0.0.1", 12355);
  DEBUGLOG("create address [%s]", addrPtr->ToString().c_str());
  myRocketRPC::TcpClient tcpClient(addrPtr);
  tcpClient.ConnectServer([addrPtr, &tcpClient]()
                          { DEBUGLOG("success connect to [%s]", addrPtr->ToString().c_str());

  // test stringProtocol
  // std::shared_ptr<myRocket::StringProtocol> message = std::make_shared<myRocket::StringProtocol>();
  // message->myMessageID = "123456789";
  // message->info = "hello, myRocket";
  
  // test tinypb
  std::shared_ptr<myRocketRPC::TinyProtocol> message = std::make_shared<myRocketRPC::TinyProtocol>();
  message->myMessageID = "123456789";
  message->myPBData = "hello, myRocket";
  DEBUGLOG("message id = [%s]", message->myMessageID.c_str());
  DEBUGLOG("my pb data = [%s]", message->myPBData.c_str());
  tcpClient.WriteMessage(message, [](myRocketRPC::AbstractProtocol::myAbstractProtocolPtr msgPtr)
                         { DEBUGLOG("send message success"); });

  tcpClient.ReadMessage(message->myMessageID, [](myRocketRPC::AbstractProtocol::myAbstractProtocolPtr msgPtr) {
    
    // std::shared_ptr<myRocket::StringProtocol> message = std::dynamic_pointer_cast<myRocket::StringProtocol>(msgPtr);
    // DEBUGLOG("messageID[%s], get response [%s]", message->myMessageID.c_str(), message->info.c_str());
    std::shared_ptr<myRocketRPC::TinyProtocol> message = std::dynamic_pointer_cast<myRocketRPC::TinyProtocol>(msgPtr);
    DEBUGLOG("messageID[%s], get response [%s]", message->myMessageID.c_str(), message->myPBData.c_str());
  }); });
}

int main()
{
  myRocketRPC::Config::SetGlobalConfig("/home/luncles/myRocketRPC/conf/myRocket.xml");
  myRocketRPC::Logger::InitGlobalLogger(1);

  // test_connect();
  // test_connect_client();
  test_tcp_client();
  return 0;
}