/* ************************************************************************
> File Name:     string_coder.h
> Author:        Luncles
> 功能:           对 抽象协议对象-字节流 进行编解码的子类
> Created Time:  2023年08月08日 星期二 20时36分07秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_CODER_STRING_CODER_H
#define MYROCKETRPC_NET_CODER_STRING_CODER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <memory>
#include <vector>
#include <random>
#include "abstract_coder.h"
#include "abstract_protocol.h"
#include "myRocketRPC/net/tcp/tcp_buffer.h"

namespace myRocketRPC
{

  class StringProtocol : public AbstractProtocol
  {
  public:
    std::string info = "hello, myRocket";
  };

  class StringCoder : public AbstractCoder
  {
  public:
    // 将抽象协议对象转换为字节流，写入到sendbuffer中
    void Encode(std::vector<AbstractProtocol::myAbstractProtocolPtr> &sendMessage, TCPBuffer::myTCPBufferPtr sendBuffer)
    {
      for (size_t i = 0; i < sendMessage.size(); i++)
      {
        // 先将基类指针转为子类指针
        std::shared_ptr<StringProtocol> sendMsgPtr = std::dynamic_pointer_cast<StringProtocol>(sendMessage[i]);
        sendBuffer->WriteToBuffer(sendMsgPtr->info.c_str(), sendMsgPtr->info.length());
      }
    }

    // 将recvbuffer中的字节流转换为抽象协议对象
    void Decode(std::vector<AbstractProtocol::myAbstractProtocolPtr> &recvMessage, TCPBuffer::myTCPBufferPtr recvBuffer)
    {
      std::vector<char> tmpVec;
      recvBuffer->ReadFromBuffer(tmpVec, recvBuffer->ReadRemain());
      std::string info;
      for (int i = 0; i < tmpVec.size(); i++)
      {
        info += tmpVec[i];
      }

      std::shared_ptr<StringProtocol> recvMsgPtr = std::make_shared<StringProtocol>();
      recvMsgPtr->info = info;
      // std::random_device rd;
      // recvMsgPtr->myMessageID = std::to_string(rd());
      recvMsgPtr->myMessageID = {"123456789"};
      recvMessage.push_back(recvMsgPtr);
    }

  private:
  };
}

#endif
