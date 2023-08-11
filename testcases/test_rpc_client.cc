/* ************************************************************************
> File Name:     test_rpc_client.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月10日 星期四 22时37分47秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include "/home/luncles/myRocketRPC/common/config.h"
#include "/home/luncles/myRocketRPC/common/log.h"
#include "/home/luncles/myRocketRPC/net/eventloop.h"
#include "/home/luncles/myRocketRPC/net/fd_event.h"
#include "/home/luncles/myRocketRPC/net/timer.h"
#include "/home/luncles/myRocketRPC/net/io_thread.h"
#include "/home/luncles/myRocketRPC/net/io_thread_group.h"
#include "../net/tcp/tcp_connection.h"
#include "../net/tcp/tcp_client.h"
#include "../net/coder/string_coder.h"
#include "../net/coder/tinypb_coder.h"
#include "../net/coder/tinypb_protocol.h"
#include "order.pb.h"

void test_tcp_client()
{
  myRocket::IPNetAddr::myNetAddrPtr addrPtr = std::make_shared<myRocket::IPNetAddr>("127.0.0.1", 12355);
  DEBUGLOG("create address [%s]", addrPtr->ToString().c_str());
  myRocket::TcpClient tcpClient(addrPtr);
  tcpClient.ConnectServer([addrPtr, &tcpClient]()
                          { DEBUGLOG("success connect to [%s]", addrPtr->ToString().c_str());

  // test stringProtocol
  // std::shared_ptr<myRocket::StringProtocol> message = std::make_shared<myRocket::StringProtocol>();
  // message->myMessageID = "123456789";
  // message->info = "hello, myRocket";
  
  // test tinypb
  std::shared_ptr<myRocket::TinyProtocol> message = std::make_shared<myRocket::TinyProtocol>();
  message->myMessageID = "123456789";
  // message->myPBData = "hello, myRocket";

  // 在客户端这边可以传rpc的参数，以便调用相关的方法
  makeOrderRequest request;
  request.set_price(100);
  request.set_goods("ffff");

  //序列化protobuf数据，将要request序列化后放到message的myPBdata里
  if (!request.SerializeToString(&(message->myPBData))) {
    ERRORLOG("serialize error");
    return;
  }

  // 注明要调用的方法
  message->myMethodName = "Order.makeOrder";

  DEBUGLOG("message id = [%s]", message->myMessageID.c_str());
  DEBUGLOG("my pb data = %s", message->myPBData.c_str());
  DEBUGLOG("myMethodName = [%s]", message->myMethodName.c_str());
  tcpClient.WriteMessage(message, [request](myRocket::AbstractProtocol::myAbstractProtocolPtr msgPtr)
                         { DEBUGLOG("send message success, request[%s]", request.ShortDebugString().c_str()); });

  tcpClient.ReadMessage(message->myMessageID, [](myRocket::AbstractProtocol::myAbstractProtocolPtr msgPtr) {
    
    // std::shared_ptr<myRocket::StringProtocol> message = std::dynamic_pointer_cast<myRocket::StringProtocol>(msgPtr);
    // DEBUGLOG("messageID[%s], get response [%s]", message->myMessageID.c_str(), message->info.c_str());
    std::shared_ptr<myRocket::TinyProtocol> message = std::dynamic_pointer_cast<myRocket::TinyProtocol>(msgPtr);
    DEBUGLOG("messageID[%s], get response [%s]", message->myMessageID.c_str(), message->myPBData.c_str());
    makeOrderResponse response;


    if (!response.ParseFromString(message->myPBData)) {
      ERRORLOG("deserialize error!");
      return;
    }
    DEBUGLOG("success get response from rpc, response[%s]", response.ShortDebugString().c_str());
  }); });
}

int main()
{
  myRocket::Config::SetGlobalConfig("/home/luncles/myRocketRPC/conf/myRocket.xml");
  myRocket::Logger::InitGlobalLogger(1);

  // test_connect();
  // test_connect_client();
  test_tcp_client();
  return 0;
}
