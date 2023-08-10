/* ************************************************************************
> File Name:     abstract_coder.h
> Author:        Luncles
> 功能:          对抽象协议对象进行编解码的抽象基类
> Created Time:  2023年08月08日 星期二 20时26分39秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_CODER_ABSTRACT_CODER_H
#define MYROCKETRPC_NET_CODER_ABSTRACT_CODER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "abstract_protocol.h"
#include "../tcp/tcp_buffer.h"

namespace myRocket
{
  struct AbstractCoder
  {
  public:
    // 将抽象协议对象转换为字节流，写入到sendbuffer中
    virtual void Encode(std::vector<AbstractProtocol::myAbstractProtocolPtr> &sendMessage, TCPBuffer::myTCPBufferPtr sendBuffer) = 0;

    // 将recvbuffer中的字节流转换为抽象协议对象
    virtual void Decode(std::vector<AbstractProtocol::myAbstractProtocolPtr> &recvMessage, TCPBuffer::myTCPBufferPtr recvBuffer) = 0;

    virtual ~AbstractCoder() {}

  private:
  };
}

#endif