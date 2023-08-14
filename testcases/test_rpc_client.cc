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
#include "../net/rpc/rpc_channel.h"
#include "../net/rpc/rpc_controller.h"
#include "../net/rpc/rpc_closure.h"
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

void test_rpc_channel()
{
  myRocket::IPNetAddr::myNetAddrPtr addrPtr = std::make_shared<myRocket::IPNetAddr>("127.0.0.1", 12355);
  DEBUGLOG("create address [%s]", addrPtr->ToString().c_str());

  // NEWRPCCHANNEL("127.0.0.1:12355", channel);
  std::shared_ptr<myRocket::RpcChannel> channel = std::make_shared<myRocket::RpcChannel>(addrPtr);

  // request
  std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();
  // NEWMESSAGE(makeOrderRequest, request);
  request->set_price(100);
  request->set_goods("apples");
  // response
  // NEWMESSAGE(makeOrderResponse, response);
  std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();

  // controller
  // NEWRPCCONTROLLER(rpcController);
  std::shared_ptr<myRocket::RpcController> rpcController = std::make_shared<myRocket::RpcController>();
  // rpcController->SetMessageID("999999999");
  rpcController->SetTimeout(10000);
  // done

  std::shared_ptr<myRocket::RpcClosure> closure = std::make_shared<myRocket::RpcClosure>([request, response, channel, rpcController]() mutable
                                                                                         {
    if (rpcController->GetErrorCode() == 0) {
    INFOLOG("call rpc success in callback, request[%s], response[%s]", request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
    }
    else {
    INFOLOG("call rpc failed in callback, request[%s], errno=[%d], error=[%s]",
    request->ShortDebugString().c_str(), rpcController->GetErrorCode(), rpcController->GetErrorInfo().c_str());
    }
    INFOLOG("now exit eventloop");
    //channel->GetTcpClient()->Stop();
    channel.reset(); });

  // rpcController->SetTimeout(10000);
  channel->Init(rpcController, request, response, closure);

  // 客户端的存根，里面保存着在proto文件里定义的rpc方法
  // 原型：Order_Stub(::PROTOBUF_NAMESPACE_ID::RpcChannel* channel);
  Order_Stub stub(channel.get());
  // 这里调用makeOrder其实就是调用了CallMethod方法，所以需要那四个参数
  stub.makeOrder(rpcController.get(), request.get(), response.get(), closure.get());
  // CALLRPC("127.0.0.1:12355", Order_Stub, makeOrder, rpcController, request, response, closure);
}

int main()
{
  myRocket::Config::SetGlobalConfig(nullptr);
  myRocket::Logger::InitGlobalLogger(0);

  // test_connect();
  // test_connect_client();
  // test_tcp_client();
  test_rpc_channel();
  return 0;
}
