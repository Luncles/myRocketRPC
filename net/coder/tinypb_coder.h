/* ************************************************************************
> File Name:     tinypb_coder.h
> Author:        Luncles
> 功能:          对"tinypb抽象协议对象-字节流"进行编解码的子类
> Created Time:  2023年08月09日 星期三 19时32分03秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_CODER_TINYPB_CODER_H
#define MYROCKETRPC_NET_CODER_TINYPB_CODER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "abstract_coder.h"
#include "tinypb_protocol.h"

namespace myRocket
{
#define TINYPB_MSG_ID_SIZE 128
#define TINYPB_METHOD_NAME_SIZE 512
#define TINYPB_ERROR_INFO_SIZE 512
  class TinyPBCoder : public AbstractCoder
  {
  public:
    // 继承自AbstractCoder，要实现encode和decode两个函数
    TinyPBCoder() {}
    ~TinyPBCoder() {}

    // 将tinypb抽象协议对象转化为字节流，写入到sendbuffer中
    void Encode(std::vector<AbstractProtocol::myAbstractProtocolPtr> &sendMessage, TCPBuffer::myTCPBufferPtr sendBuffer);

    // 将recvbuffer中的字节流转换为tinypb抽象协议对象
    void Decode(std::vector<AbstractProtocol::myAbstractProtocolPtr> &recvMessage, TCPBuffer::myTCPBufferPtr recvBuffer);

  private:
    const char *EncodeTinyPB(std::shared_ptr<TinyProtocol> message, int &len);
  };
}
#endif