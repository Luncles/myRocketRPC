/* ************************************************************************
> File Name:     rpc_dispatcher.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月10日 星期四 16时05分17秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <vector>
#include "rpc_dispatcher.h"
#include "../../common/log.h"
#include "../../common/error_code.h"
#include "rpc_controller.h"
#include "../../common/run_time.h"

namespace myRocket
{
#define DELETE_RESOURCE(XX) \
  if (XX != nullptr)        \
  {                         \
    delete XX;              \
    XX = nullptr;           \
  }

  static RpcDispatcher *rpcDispatcher = nullptr;
  RpcDispatcher *RpcDispatcher::GetRpcDispatcher()
  {
    if (rpcDispatcher)
    {
      return rpcDispatcher;
    }
    rpcDispatcher = new RpcDispatcher;
    return rpcDispatcher;
  }

  void RpcDispatcher::Dispatch(AbstractProtocol::myAbstractProtocolPtr request, AbstractProtocol::myAbstractProtocolPtr response, TcpConnection *connection)
  {
    // 2、从recvBuffer中读取数据，进行Decode后得到请求的TinyProtocol对象。然后从请求的TinyProtocol对象中得到methodName，从OrderService里根据service.methodName找到对应的方法func。
    std::shared_ptr<TinyProtocol> requestProtocol = std::dynamic_pointer_cast<TinyProtocol>(request);
    std::shared_ptr<TinyProtocol> responseProtocol = std::dynamic_pointer_cast<TinyProtocol>(response);

    std::string fullMethodName = requestProtocol->myMethodName;
    std::string serviceName;
    std::string methodName;

    responseProtocol->myMessageID = requestProtocol->myMessageID;
    responseProtocol->myMethodName = requestProtocol->myMethodName;

    // 进行方法名的拆分
    if (!ParseServiceFullName(fullMethodName, serviceName, methodName))
    {
      SetTinyPBError(responseProtocol, ERROR_PARSE_SERVICE_NAME, "parse service name error!");
      return;
    }

    auto it = myServiceMap.find(serviceName);
    if (it == myServiceMap.end())
    {
      ERRORLOG("[%s] | service name[%s] not found", responseProtocol->myMessageID.c_str(), serviceName.c_str());
      SetTinyPBError(responseProtocol, ERROR_SERVICE_NOT_FOUND, "service not found");
      return;
    }

    protobuf_service_s_ptr service = (*it).second;

    // 找到与方法名对应的描述符
    const google::protobuf::MethodDescriptor *method = service->GetDescriptor()->FindMethodByName(methodName);
    if (!method)
    {
      ERRORLOG("[%s] | method name[%s] not found in service[%s]", responseProtocol->myMessageID.c_str(), methodName.c_str(), serviceName.c_str());
      SetTinyPBError(responseProtocol, ERROR_METHOD_NOT_FOUND, "mothed not found");
      return;
    }

    // 3、找到与方法名描述符对应的request type以及response type
    google::protobuf::Message *requestMessage = service->GetRequestPrototype(method).New();

    // 4、将请求体TinyProtocol里面的pbdata反序列化为request type对象，声明一个空的response type对象。
    if (!requestMessage->ParseFromString(requestProtocol->myPBData))
    {
      ERRORLOG("[%s] | deserilize error, method=[%s], service=[%s]", requestProtocol->myMessageID.c_str(), methodName.c_str(), serviceName.c_str());
      SetTinyPBError(responseProtocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
      DELETE_RESOURCE(requestMessage);
      return;
    }

    INFOLOG("[%s] | get rpc request[%s]", requestProtocol->myMessageID.c_str(), requestMessage->ShortDebugString().c_str());

    google::protobuf::Message *responseMessage = service->GetResponsePrototype(method).New();

    // 5、func(request, response)
    RpcController *rpcController = new RpcController();
    rpcController->SetLocalAddress(connection->GetServerAddr());
    rpcController->SetPeerAddress(connection->GetClientAddr());
    rpcController->SetMessageID(requestProtocol->myMessageID);

    RunTime::GetRunTime()->messageID = requestProtocol->myMessageID;
    RunTime::GetRunTime()->methodName = methodName;

    service->CallMethod(method, rpcController, requestMessage, responseMessage, nullptr);

    // 6、将response对象序列化为pbdata，再塞入到TinyProtocol对象中，做Encode后放到sendBuffer中，就可以发送回包了。
    if (!responseMessage->SerializeToString(&(responseProtocol->myPBData)))
    {
      ERRORLOG("[%s] | serialize error, origin message [%s]", requestProtocol->myMessageID.c_str(), responseMessage->ShortDebugString().c_str());
      SetTinyPBError(responseProtocol, ERROR_FAILED_SERIALIZE, "serialize error");
      DELETE_RESOURCE(requestMessage);
      DELETE_RESOURCE(responseMessage);
    }
    else
    {
      responseProtocol->myErrorCode = 0;
      responseProtocol->myErrorInfo = "";
      INFOLOG("[%s] | dispatch success, request[%s], response[%s]", requestProtocol->myMessageID.c_str(), requestMessage->ShortDebugString().c_str(), responseMessage->ShortDebugString().c_str());
      DELETE_RESOURCE(requestMessage);
      DELETE_RESOURCE(responseMessage);
    }

    // std::vector<AbstractProtocol::myAbstractProtocolPtr> replyMessage;
    // replyMessage.push_back(responseProtocol);
  }

  void RpcDispatcher::RegisterService(protobuf_service_s_ptr service)
  {
    std::string serviceName = service->GetDescriptor()->full_name();
    myServiceMap[serviceName] = service;
  }

  void RpcDispatcher::SetTinyPBError(std::shared_ptr<TinyProtocol> msg, int32_t errorCode, std::string errorInfo)
  {
    msg->myErrorCode = errorCode;
    msg->myErrorInfo = errorInfo;
    msg->myErrorInfoLen = errorInfo.length();
  }

  bool RpcDispatcher::ParseServiceFullName(const std::string &fullName, std::string &serviceName, std::string &methodName)
  {
    if (fullName.empty())
    {
      ERRORLOG("full name empty");
      return false;
    }

    size_t index = fullName.find_first_of(".");
    if (index == fullName.npos)
    {
      ERRORLOG("not find . in full name [%s]", fullName.c_str());
      return false;
    }

    serviceName = fullName.substr(0, index);
    methodName = fullName.substr(index + 1, fullName.length() - index - 1);

    INFOLOG("parse service_name[%s] and method[%s] from full name[%s]", serviceName.c_str(), methodName.c_str(), fullName.c_str());
    return true;
  }
}