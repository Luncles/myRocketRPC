/* ************************************************************************
> File Name:     tcp_buffer.h
> Author:        Luncles
> 功能:          应用层TCP的缓冲区，可以用来处理TCP粘包，分包等情况
> Created Time:  2023年08月02日 星期三 22时57分20秒
> Description:
 ************************************************************************/
#ifndef MYROCKETRPC_NET_TCP_TCP_BUFFER_H
#define MYROCKETRPC_NET_TCP_TCP_BUFFER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>
#include <memory>

namespace myRocket
{
  class TCPBuffer
  {
  public:
    using myTCPBufferPtr = std::shared_ptr<TCPBuffer>;

    TCPBuffer(int size);

    ~TCPBuffer();

    // 剩余可读字节数
    size_t ReadRemain();

    // 剩余可写字节数
    size_t WriteRemain();

    // 获取读指针
    size_t GetReadIndex();

    // 获取写指针
    size_t GetWriteIndex();

    // 向buffer中写入数据
    size_t WriteToBuffer(const char *src, size_t size);

    // 从buffer中读取数据
    size_t ReadFromBuffer(std::vector<char> &dest, size_t size);

    // 修改缓冲区大小
    int ResizeBuffer(size_t newSize);

    // 调整缓冲区，删除已经读取的内容以扩大可写内存
    int AdjustBuffer();

    // 移动读指针
    int MoveReadIndex(size_t size);

    // 移动写指针
    int MoveWriteIndex(size_t size);

  private:
    // 获取缓存区容量
    size_t GetBufferSize();

  private:
    size_t myReadIndex{0};  // 读指针
    size_t myWriteIndex{0}; // 写指针
    size_t mySize{0};       // buffer的容量

  public:
    std::vector<char> myBuffer; // TCP缓冲区
  };
}

#endif