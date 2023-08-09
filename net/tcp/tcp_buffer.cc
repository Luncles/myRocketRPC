/* ************************************************************************
> File Name:     tcp_buffer.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月03日 星期四 16时04分35秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tcp_buffer.h"
#include "/home/luncles/myRocketRPC/common/log.h"

namespace myRocket
{
  TCPBuffer::TCPBuffer(int size) : mySize(size)
  {
    myBuffer.resize(size);
  }

  TCPBuffer::~TCPBuffer()
  {
  }

  // 剩余可读字节数
  size_t TCPBuffer::ReadRemain()
  {
    return myWriteIndex - myReadIndex;
  }

  // 剩余可写字节数
  size_t TCPBuffer::WriteRemain()
  {
    return mySize - myWriteIndex;
  }

  // 获取读指针
  size_t TCPBuffer::GetReadIndex()
  {
    return myReadIndex;
  }

  // 获取写指针
  size_t TCPBuffer::GetWriteIndex()
  {
    return myWriteIndex;
  }

  // 向buffer中写入数据
  size_t TCPBuffer::WriteToBuffer(const char *src, size_t size)
  {
    // 先判断剩余空间够不够写，不够的话要进行扩容
    if (size > WriteRemain())
    {
      // 按2倍进行扩容
      size_t newSize = 2 * (myWriteIndex + size);
      ResizeBuffer(newSize);
    }

    // 拷贝数据
    memcpy(&myBuffer[myWriteIndex], src, size);
    myWriteIndex += size;

    // 返回写入的字节数
    return size;
  }

  // 从buffer中读取数据
  size_t TCPBuffer::ReadFromBuffer(std::vector<char> &dest, size_t size)
  {
    if (ReadRemain() == 0)
    {
      return 0;
    }
    // 先判断有没有size字节的数据可以读
    int readAble = ReadRemain() > size ? size : ReadRemain();

    // 拷贝数据
    std::vector<char> tmp(readAble);
    memcpy(&tmp[0], &myBuffer[myReadIndex], readAble);

    dest.swap(tmp);
    myReadIndex += readAble;

    // 看情况调整缓冲区
    AdjustBuffer();
    return readAble;
  }

  // 修改缓冲区大小
  int TCPBuffer::ResizeBuffer(size_t newSize)
  {
    std::vector<char> tmp(newSize);

    // 要先将缓冲区中的数据拷贝过来
    size_t count = std::min(ReadRemain(), newSize);

    memcpy(&tmp[0], &myBuffer[myReadIndex], count);

    myBuffer.swap(tmp);

    myReadIndex = 0;
    myWriteIndex = myReadIndex + count;
    mySize = newSize;
    tmp.clear();
    return 1;
  }

  // 调整缓冲区，删除已经读取的内容以扩大可写内存
  int TCPBuffer::AdjustBuffer()
  {
    // 已读字节超过容量的1/3才需要进行调整
    if (myReadIndex < mySize / 3)
    {
      return 0;
    }

    std::vector<char> tmp(mySize);

    size_t count = ReadRemain();

    memcpy(&tmp[0], &myBuffer[myReadIndex], count);
    myBuffer.swap(tmp);
    myReadIndex = 0;
    myWriteIndex = myReadIndex + count;
    tmp.clear();
    return 1;
  }

  // 移动读指针
  int TCPBuffer::MoveReadIndex(size_t size)
  {
    if (size > ReadRemain())
    {
      ERRORLOG("Move read index error, invalid size [%d], old read index [%d], buffer size [%d], readRemain [%d]", size, myReadIndex, mySize, ReadRemain());
      return -1;
    }
    myReadIndex = myReadIndex + size;
    AdjustBuffer();
    return 1;
  }

  // 移动写指针
  int TCPBuffer::MoveWriteIndex(size_t size)
  {
    if (size >= WriteRemain())
    {
      ERRORLOG("Move write index error, invalid size [%d], old write index [%d], buffer size [%d]", size, myWriteIndex, mySize);
      return -1;
    }
    myWriteIndex = myWriteIndex + size;
    AdjustBuffer();
    return 1;
  }

  size_t TCPBuffer::GetBufferSize()
  {
    return mySize;
  }
}