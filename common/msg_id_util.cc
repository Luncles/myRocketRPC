/* ************************************************************************
> File Name:     msg_id_util.cc
> Author:        Luncles
> 功能:
> Created Time:  2023年08月11日 星期五 16时33分41秒
> Description:
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "msg_id_util.h"
#include "log.h"

namespace myRocketRPC
{
  static int messageIDLen = 20; // message id的长度
  static int randomFD = -1;     // 随机文件的句柄

  static thread_local std::string messageIDNo;    // 当前的message id
  static thread_local std::string maxMessageIDNo; // 最大的message id

  std::string MsgIDUtil::MsgIDGenerator()
  {
    if (messageIDNo.empty() || messageIDNo == maxMessageIDNo)
    {
      if (randomFD == -1)
      {
        // 随机找一个文件句柄
        randomFD = open("/dev/urandom", O_RDONLY);
      }
      std::string res(messageIDLen, 0);
      if ((read(randomFD, &res[0], messageIDLen)) != messageIDLen)
      {
        ERRORLOG("read from /dev/urandom error");
        return "";
      }

      // 因为读到的随机序列res不一定都是可见字符的，所以要做一个映射
      for (int i = 0; i < messageIDLen; i++)
      {
        uint8_t x = ((uint8_t)(res[i])) % 10;
        res[i] = x + '0';
        maxMessageIDNo += '9'; // 因为表示的是最大的message id，所以最大的就是“9999999.。。。”
      }

      messageIDNo = res;
    }
    else
    {
      // 不用生成随机数的，直接就进行递增
      size_t i = messageIDNo.length() - 1; // 先找到个位数
      while (messageIDNo[i] == '9' && i >= 0)
      { // 如果最后一位是9，加1则向前进位
        i--;
      }
      if (i >= 0)
      {
        messageIDNo[i] += 1;
        // 后面的位数全部置零
        for (size_t j = i + 1; j < messageIDNo.length(); j++)
        {
          messageIDNo[j] = '0';
        }
      }
    }

    return messageIDNo;
  }
}
