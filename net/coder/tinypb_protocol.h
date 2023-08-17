/* ************************************************************************
> File Name:     tinypb_protocol.h
> Author:        Luncles
> 功能:          基于protobuf的自定义协议
> Created Time:  2023年08月09日 星期三 19时32分18秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_CODER_TINYPB_PROTOCOL_H
#define MYROCKETRPC_NET_CODER_TINYPB_PROTOCOL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "abstract_protocol.h"

namespace myRocketRPC
{
  struct TinyProtocol : public AbstractProtocol
  {
  public:
    TinyProtocol() {}
    ~TinyProtocol() {}

  public:
    static char PB_START; // 开始符，一个字节
    static char PB_END;   // 结束符，一个字节

    int32_t myPackageLen{0};   // 整包长度，四个字节
    int32_t myMessageIDLen{0}; // messageid的长度，4字节

    // myMessageID继承自父类
    // std::string myMessageID; // 请求号，唯一标识一个请求/响应

    int32_t myMethodNameLen{0}; // 方法名长度，4字节
    std::string myMethodName;   // 方法名
    int32_t myErrorCode{0};     // 错误码
    int32_t myErrorInfoLen{0};  // 错误信息长度
    std::string myErrorInfo;    // 错误信息
    std::string myPBData;       // Protobuf序列化数据
    int32_t myCheckSum{0};      // 校验码

    bool myParseSuccess{false}; // 读到完整包的标志
  };
}

#endif