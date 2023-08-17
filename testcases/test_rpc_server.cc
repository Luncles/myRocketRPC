/* ************************************************************************
> File Name:     test_rpc_server.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月10日 星期四 22时49分15秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include <google/protobuf/service.h>
#include "myRocketRPC/common/config.h"
#include "myRocketRPC/common/log.h"
#include "myRocketRPC/net/eventloop.h"
#include "myRocketRPC/net/fd_event.h"
#include "myRocketRPC/net/timer.h"
#include "myRocketRPC/net/io_thread.h"
#include "myRocketRPC/net/io_thread_group.h"
#include "myRocketRPC/net/tcp/tcp_connection.h"
#include "myRocketRPC/net/tcp/tcp_server.h"
#include "myRocketRPC/net/coder/string_coder.h"
#include "myRocketRPC/net/coder/tinypb_coder.h"
#include "myRocketRPC/net/coder/tinypb_protocol.h"
#include "myRocketRPC/net/rpc/rpc_dispatcher.h"
#include "myRocketRPC/common/log.h"
#include "order.pb.h"
#include "myRocketRPC/common/run_time.h"

// RPC的用法：用一个派生类去继承proto文件里生成的RPC服务接口类，在派生类中去实现接口类的虚函数
class OrderImpl : public Order
{
public:
  void makeOrder(google::protobuf::RpcController *controller,
                 const ::makeOrderRequest *request,
                 ::makeOrderResponse *response,
                 ::google::protobuf::Closure *done)
  {
    APPDEBUGLOG("start sleep 5s");
    sleep(5);
    APPDEBUGLOG("end sleep 5s");
    // 先写一个简单的例子，这里的request就包含了proto文件里订单请求结构体中定义的成员，response包含了proto文件里订单响应结构体中定义的成员
    if (request->price() < 10)
    {
      response->set_returncode(-1);
      response->set_returninfo("not enough money");
      return;
    }
    else
    {
      response->set_orderid("20230810");
    }
    if (done)
    {
      done->Run();
      delete done;
      done = nullptr;
    }
    APPDEBUGLOG("call makeOrder method end");
  }

private:
};

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Start test_rpc_server error, argc not 2 \n");
    printf("Start like this: \n");
    printf("./test_rpc_server ../conf/myRocket.xml \n");
    return 0;
  }
  myRocketRPC::Config::SetGlobalConfig(argv[1]);
  myRocketRPC::Logger::InitGlobalLogger(1);

  // 下面是把OrderImpl类添加到rpc调用中的过程
  // 先定义一个OrderImpl类的服务指针
  std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();

  // 然后注册到分发器里
  myRocketRPC::RpcDispatcher::GetRpcDispatcher()->RegisterService(service);

  // 接下来就是启动服务器了
  myRocketRPC::IPNetAddr::myNetAddrPtr addrPtr = std::make_shared<myRocketRPC::IPNetAddr>("127.0.0.1", myRocketRPC::Config::GetGlobalConfig()->myPort);
  DEBUGLOG("create address [%s]", addrPtr->ToString().c_str());
  myRocketRPC::TCPServer tcpServer(addrPtr);
  tcpServer.Start();

  return 0;
}