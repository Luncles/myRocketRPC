/* ************************************************************************
> File Name:     rpc_channel.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月11日 星期五 16时57分06秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "rpc_channel.h"
#include "../../common/log.h"
#include "../../common/error_code.h"
#include "../../common/msg_id_util.h"
#include "../coder/tinypb_protocol.h"
#include "rpc_controller.h"

namespace myRocket
{
  RpcChannel::RpcChannel(NetAddr::myNetAddrPtr peerAddr) : myPeerAddr(peerAddr)
  {
    INFOLOG("RpcChannel");
  }

  RpcChannel::~RpcChannel()
  {
    INFOLOG("~RpcChannel");
  }

  // 调用远程服务的方法
  // Call the given method of the remote service.  The signature of this
  // procedure looks the same as Service::CallMethod(), but the requirements
  // are less strict in one important way:  the request and response objects
  // need not be of any specific class as long as their descriptors are
  // method->input_type() and method->output_type().
  void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                              google::protobuf::Message *response, google::protobuf::Closure *done)
  {
    std::shared_ptr<TinyProtocol> requestPtl = std::make_shared<TinyProtocol>();

    RpcController *my_controller = dynamic_cast<RpcController *>(controller);

    if (!my_controller || !request || !response)
    {
      ERRORLOG("failed to callmethod, RpcController convert error!");
      my_controller->SetError(ERROR_RPC_CHANNEL_INIT, "controller or request or response nullptr");

      // 执行rpc调用回调函数
      myCallBack();
      return;
    }

    myTcpClient = std::make_shared<TcpClient>(myPeerAddr);

    // 要做的几步：1、拿到一个tinypb消息包；2、将message封进消息包里；3、连接服务器，这时候需要配置tcpclient；4、write消息包；5、read响应
    if (my_controller->GetMessageID().empty())
    {

      // tinypb消息包需要一个message id，没有的话就生成一个
      requestPtl->myMessageID = MsgIDUtil::MsgIDGenerator();
      my_controller->SetMessageID(requestPtl->myMessageID);
    }
    else
    {
      // controller中有的话直接赋值
      requestPtl->myMessageID = my_controller->GetMessageID();
    }

    // 接下来赋值方法
    requestPtl->myMethodName = method->full_name();
    INFOLOG("[%s] | call method name [%s]", requestPtl->myMessageID.c_str(), requestPtl->myMethodName.c_str());

    // 检查有没有保存好proto资源
    if (!myIsInit)
    {
      std::string errorInfo = "RpcChannel not call init()";
      my_controller->SetError(ERROR_RPC_CHANNEL_INIT, errorInfo);
      ERRORLOG("[%s] | [%s], RpcChannel not init", requestPtl->myMessageID.c_str(), errorInfo.c_str());
      myCallBack();
      return;
    }
    // 接下来将request序列化后，放进消息包的myPBData里
    if (!request->SerializeToString(&(requestPtl->myPBData)))
    {
      std::string errorInfo = "failed to serialize";
      my_controller->SetError(ERROR_FAILED_SERIALIZE, errorInfo);
      ERRORLOG("[%s] | [%s], origin request [%s]", requestPtl->myMessageID.c_str(), errorInfo.c_str(), request->ShortDebugString().c_str());
      myCallBack();
      return;
    }

    // 拿到智能指针
    myRpcChannelPtr channel = shared_from_this();

    // 连接服务器
    myTcpClient->ConnectServer([requestPtl, this, channel]() mutable
                               {
      RpcController* my_controller = dynamic_cast<RpcController*>(GetController());

      if (GetTcpClient()->GetConnectErrorCode() != 0) {
        my_controller->SetError(GetTcpClient()->GetConnectErrorCode(), GetTcpClient()->GetConnectErrorInfo());
        ERRORLOG("[%s] | connect error, error code=[%d], error info=[%s], peer address=[%s]",
        requestPtl->myMessageID.c_str(), my_controller->GetErrorCode(), 
        my_controller->GetErrorInfo().c_str(), GetTcpClient()->GetServerAddress()->ToString().c_str());

        myCallBack();
        return;
      }

      // 执行到这里表示连接成功了
      INFOLOG("[%s] | connect success, peer address[%s], local address[%s]", 
      requestPtl->myMessageID.c_str(),
      GetTcpClient()->GetServerAddress()->ToString().c_str(),
      GetTcpClient()->GetLocalAddress()->ToString().c_str());

      // 接下来要发送数据了
      GetTcpClient()->WriteMessage(requestPtl, [requestPtl, this, my_controller, channel](AbstractProtocol::myAbstractProtocolPtr) mutable {
        INFOLOG("[%s] | send rpc request success. Call method[%s], peer address[%s], local address[%s]", 
        requestPtl->myMessageID.c_str(), requestPtl->myMethodName.c_str(),
        GetTcpClient()->GetServerAddress()->ToString().c_str(),
        GetTcpClient()->GetLocalAddress()->ToString().c_str());

        // 发送完数据要读回包
        GetTcpClient()->ReadMessage(requestPtl->myMessageID, [this, my_controller, channel](AbstractProtocol::myAbstractProtocolPtr msg) mutable {
          std::shared_ptr<TinyProtocol> responsePtl = std::dynamic_pointer_cast<TinyProtocol>(msg);
          INFOLOG("[%s] | get rpc response success. Call method[%s], peer address[%s], local address[%s]", 
          responsePtl->myMessageID.c_str(), responsePtl->myMethodName.c_str(),
          GetTcpClient()->GetServerAddress()->ToString().c_str(),
          GetTcpClient()->GetLocalAddress()->ToString().c_str());

          // 解析回包数据
          if (!(GetResponse()->ParseFromString(responsePtl->myPBData))) {
            ERRORLOG("[%s] | serialize error", responsePtl->myMessageID.c_str());
            my_controller->SetError(ERROR_FAILED_DESERIALIZE, "deserialize error");
            myCallBack();
            return;
          }

          if (responsePtl->myErrorCode != 0) {
            ERRORLOG("[%s] | call rpc method[%s] failed, error code[%d], error info[%s]",
            responsePtl->myMethodName.c_str(), responsePtl->myErrorCode,
            responsePtl->myErrorInfo.c_str());
            my_controller->SetError(responsePtl->myErrorCode, responsePtl->myErrorInfo);
            myCallBack();
            return;
          }

          // 到这里就是rpc调用成功的情况了
          INFOLOG("[%s] | call rpc success, call method[%s], peer address[%s], local address[%s]", 
          responsePtl->myMessageID.c_str(), responsePtl->myMethodName.c_str(),
          GetTcpClient()->GetServerAddress()->ToString().c_str(),
          GetTcpClient()->GetLocalAddress()->ToString().c_str());

          channel.reset();
          myCallBack();

          
        });
      }); });
  }

  TcpClient *RpcChannel::GetTcpClient()
  {
    return myTcpClient.get();
  }

  google::protobuf::RpcController *RpcChannel::GetController()
  {
    return myController.get();
  }

  google::protobuf::Message *RpcChannel::GetRequest()
  {
    return myRequest.get();
  }

  google::protobuf::Message *RpcChannel::GetResponse()
  {
    return myResponse.get();
  }

  google::protobuf::Closure *RpcChannel::GetClosure()
  {
    return myClosure.get();
  }

  void RpcChannel::Init(myControllerPtr controller, myMessagePtr request, myMessagePtr response, myClosurePtr done)
  {
    if (myIsInit)
    {
      return;
    }
    myController = controller;
    myRequest = request;
    myResponse = response;
    myClosure = done;
    myIsInit = true;
  }

  // 执行的回调函数
  void RpcChannel::myCallBack()
  {
    RpcController *my_controller = dynamic_cast<RpcController *>(GetController());
    if (my_controller->Finished() || my_controller->GetErrorCode() != 0)
    {
      return;
    }

    // 执行rpc channel的回调函数
    if (myClosure)
    {
      myClosure->Run();
      if (my_controller)
      {
        my_controller->SetFinished(true);
      }
    }
  }

}
