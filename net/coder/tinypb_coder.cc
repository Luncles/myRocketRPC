/* ************************************************************************
> File Name:     tinypb_coder.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月09日 星期三 19时57分13秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>
#include <memory>
#include "tinypb_coder.h"
#include "tinypb_protocol.h"
#include "../../common/log.h"
#include "../../common/util.h"

namespace myRocket
{
  // 将tinypb抽象协议对象转化为字节流，写入到sendbuffer中
  void TinyPBCoder::Encode(std::vector<AbstractProtocol::myAbstractProtocolPtr> &sendMessage, TCPBuffer::myTCPBufferPtr sendBuffer)
  {
    for (auto m : sendMessage)
    {
      // 先把基类智能指针转换为派生类智能指针
      std::shared_ptr<TinyProtocol> msg = std::dynamic_pointer_cast<TinyProtocol>(m);
      int len = 0;
      const char *buf = EncodeTinyPB(msg, len);
      if (buf && len != 0)
      {
        sendBuffer->WriteToBuffer(buf, len);
      }
      if (buf)
      {
        // 因为buf的空间是malloc出来的，所以要free掉
        free((void *)buf);
        buf = nullptr;
      }
    }
  }

  // 将recvbuffer中的字节流转换为tinypb抽象协议对象
  void TinyPBCoder::Decode(std::vector<AbstractProtocol::myAbstractProtocolPtr> &recvMessage, TCPBuffer::myTCPBufferPtr recvBuffer)
  {
    while (1)
    {
      // 遍历RecvBuffer，找到PB_START，找到之后，解析出整包的长度，根据整包长度找到结束符的位置，如果目标位置确实是结束符，说明是一个完整包
      std::vector<char> tmp = recvBuffer->myBuffer;
      int startIndex = recvBuffer->GetReadIndex(); // 开始索引
      int endIndex = -1;                           // 结束索引

      int packageLen = 0;        // 包长度
      bool parseSuccess = false; // 读完整的包成功标志
      int index = 0;
      for (index = startIndex; index < recvBuffer->GetWriteIndex(); index++)
      {
        if (tmp[index] == TinyProtocol::PB_START)
        {
          // 读包开始符后面的四个字节，就是整包长度，因为是网络字节序（大端），所以要转为主机字节序（小端）
          if (index + 1 < recvBuffer->GetWriteIndex())
          {
            packageLen = GetInt32FromNetByte(&tmp[index + 1]);
            DEBUGLOG("get package len = [%d]", packageLen);

            // 结束符的索引
            int end = index + packageLen - 1;
            // 读到不完整的包
            if (end >= recvBuffer->GetWriteIndex())
            {
              continue;
            }

            // 读到完整的包
            if (tmp[end] == TinyProtocol::PB_END)
            {
              startIndex = index;
              endIndex = end;
              parseSuccess = true;
              break;
            }
          }
        }
      }

      if (index >= recvBuffer->GetWriteIndex())
      {
        DEBUGLOG("decode end, read all buffer data");
        return;
      }

      if (parseSuccess)
      {
        recvBuffer->MoveReadIndex(endIndex - startIndex + 1);
        std::shared_ptr<TinyProtocol> message = std::make_shared<TinyProtocol>();
        message->myPackageLen = packageLen;

        // 接下来检查myMessageIDLen
        int messageIDLenIndex = startIndex + sizeof(char) + sizeof(message->myPackageLen);
        if (messageIDLenIndex >= endIndex)
        {
          message->myParseSuccess = false;
          ERRORLOG("parser error, messageIDLenIndex[%d] >= endindex[%d]", messageIDLenIndex, endIndex);
          continue;
        }
        message->myMessageIDLen = GetInt32FromNetByte(&tmp[messageIDLenIndex]);
        DEBUGLOG("parse messageIDLen=[%d]", message->myMessageIDLen);

        // 移四个字节就是messageID了
        int messageIDIndex = messageIDLenIndex + sizeof(message->myMessageIDLen);

        char messageID[TINYPB_MSG_ID_SIZE] = {0};
        memcpy(&messageID[0], &tmp[messageIDIndex], message->myMessageIDLen);
        // 字符串数据在网络上传输时不需要进行字节序转换，因为它们是由单个字节组成的，不受字节序的影响
        message->myMessageID = std::string(messageID);
        DEBUGLOG("parse messageID=[%s]", message->myMessageID.c_str());

        // 接下来处理方法名长度+方法名
        int methodNameLenIndex = messageIDIndex + message->myMessageIDLen;
        if (methodNameLenIndex >= endIndex)
        {
          message->myParseSuccess = false;
          ERRORLOG("parser error, methodNameLenIndex[%d] >= endindex[%d]", methodNameLenIndex, endIndex);
          continue;
        }
        message->myMethodNameLen = GetInt32FromNetByte(&tmp[methodNameLenIndex]);
        DEBUGLOG("parse methon name length=[%d]", message->myMethodNameLen);

        // 移四个字节就是method name了
        int methodNameIndex = methodNameLenIndex + sizeof(message->myMethodNameLen);
        char methodName[TINYPB_METHOD_NAME_SIZE] = {0};
        memcpy(&methodName[0], &tmp[methodNameIndex], message->myMethodNameLen);
        message->myMethodName = std::string(methodName);
        DEBUGLOG("parse method name = [%s]", message->myMethodName.c_str());

        // 接下来处理错误码
        int errorCodeIndex = methodNameIndex + message->myMethodNameLen;
        if (errorCodeIndex >= endIndex)
        {
          message->myParseSuccess = false;
          ERRORLOG("parser error, errorCodeIndex[%d] >= endindex[%d]", errorCodeIndex, endIndex);
          continue;
        }
        message->myErrorCode = GetInt32FromNetByte(&tmp[errorCodeIndex]);

        // 接下处理错误信息长度+错误信息
        int errorInfoLenIndex = errorCodeIndex + sizeof(message->myErrorCode);
        if (errorInfoLenIndex >= endIndex)
        {
          message->myParseSuccess = false;
          ERRORLOG("parser error, errorInfoLenIndex[%d] >= endindex[%d]", errorInfoLenIndex, endIndex);
          continue;
        }
        message->myErrorInfoLen = GetInt32FromNetByte(&tmp[errorInfoLenIndex]);

        // 再移4个字节就是error info了
        int errorInfoIndex = errorInfoLenIndex + sizeof(message->myErrorInfoLen);
        char errorInfo[TINYPB_ERROR_INFO_SIZE] = {0};
        memcpy(&errorInfo, &tmp[errorInfoIndex], message->myErrorInfoLen);
        message->myErrorInfo = std::string(errorInfo);
        DEBUGLOG("parse error info = [%s]", message->myErrorInfo.c_str());

        // 接下来处理protobuf序列化数据，因为没有标记数据长度的字段，所以要用总的减去其他的
        int protobufDataLen = message->myPackageLen - 2 - message->myMethodNameLen - message->myMessageIDLen - message->myErrorInfoLen - 6 * sizeof(message->myPackageLen);
        int protobufDataIndex = errorInfoIndex + message->myErrorInfoLen;
        message->myPBData = std::string(&tmp[protobufDataIndex], protobufDataLen);
        DEBUGLOG("the message data is [%s]", message->myPBData.c_str());
        // TODO：接下来解析校验和

        // 走到这里就全部解析成功了
        message->myPackageLen = true;

        // 插入返回集合里
        recvMessage.push_back(message);
      }
    }
  }

  const char *TinyPBCoder::EncodeTinyPB(std::shared_ptr<TinyProtocol> message, int &len)
  {
    if (message->myMessageID.empty())
    {
      message->myMessageID = "123456789";
    }
    DEBUGLOG("this message ID = [%s]", message->myMessageID.c_str());
    int packageLen = 2 + message->myMessageID.length() + message->myMethodName.length() + message->myErrorInfo.length() + 6 * sizeof(message->myPackageLen) + message->myPBData.length();
    DEBUGLOG("package length =[%d]", packageLen);
    char *buf = reinterpret_cast<char *>(malloc(packageLen));
    char *tmp = buf;

    *tmp = TinyProtocol::PB_START;
    tmp++;

    // 注意将主机字节序转换为网络字节序
    int32_t packageLenNet = htonl(packageLen);
    memcpy(tmp, &packageLenNet, sizeof(packageLenNet));
    tmp += sizeof(packageLenNet);

    // 接下来处理messageID和其长度
    int messageIDLen = message->myMessageID.length();
    int32_t messageIDLenNet = htonl(messageIDLen);
    memcpy(tmp, &messageIDLenNet, sizeof(messageIDLenNet));
    tmp += sizeof(messageIDLenNet);

    if (!message->myMessageID.empty())
    {
      memcpy(tmp, &(message->myMessageID[0]), messageIDLen);
      tmp += messageIDLen;
    }

    // 接下来处理方法名长度和方法名
    int methodNameLen = message->myMethodName.length();
    int32_t methodNameLenNet = htonl(methodNameLen);
    memcpy(tmp, &methodNameLenNet, sizeof(methodNameLenNet));
    tmp += sizeof(methodNameLenNet);

    if (!message->myMethodName.empty())
    {
      memcpy(tmp, &(message->myMethodName[0]), methodNameLen);
      tmp += messageIDLen;
    }

    // 接下来处理错误码
    int32_t errorCodeNet = htonl(message->myErrorCode);
    memcpy(tmp, &errorCodeNet, sizeof(errorCodeNet));
    tmp += sizeof(errorCodeNet);

    // 接下来处理错误信息长度和错误信息
    int errorInfoLen = message->myErrorInfo.length();
    int32_t errorInfoLenNet = htonl(errorInfoLen);
    memcpy(tmp, &errorInfoLenNet, sizeof(errorInfoLenNet));
    tmp += sizeof(errorInfoLenNet);

    if (!message->myErrorInfo.empty())
    {
      memcpy(tmp, &(message->myErrorInfo[0]), errorInfoLen);
      tmp += errorInfoLen;
    }

    // 接下来处理protobuf数据序列
    if (!message->myPBData.empty())
    {
      memcpy(tmp, &(message->myPBData[0]), message->myPBData.length());
      tmp += message->myPBData.length();
    }

    // TODO:确认校验和
    int32_t checkSumNet = htonl(1);
    memcpy(tmp, &checkSumNet, sizeof(checkSumNet));
    tmp += sizeof(checkSumNet);

    *tmp = TinyProtocol::PB_END;

    message->myPackageLen = packageLen;
    message->myMessageIDLen = messageIDLen;
    message->myMethodNameLen = methodNameLen;
    message->myErrorInfoLen = errorInfoLen;
    message->myParseSuccess = true;

    len = packageLen;

    DEBUGLOG("encode message[%s] success", message->myMessageID.c_str());

    return buf;
  }

}