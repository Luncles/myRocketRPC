#include <pthread.h>

#include "myRocketRPC/common/config.h"
#include "myRocketRPC/common/log.h"

void *PthreadFunc(void *)
{
  DEBUGLOG("new thread test debug log %s", "11");
  INFOLOG("new thread test info log %s", "22");
  return nullptr;
}

int main()
{
  myRocketRPC::Config::SetGlobalConfig(
      "/home/luncles/myRocketRPC/conf/myRocket.xml");
  myRocketRPC::Logger::InitGlobalLogger(1);

  pthread_t thread;
  pthread_create(&thread, nullptr, &PthreadFunc, nullptr);

  DEBUGLOG("test debug log %s", "11");
  INFOLOG("test info log %s", "22");

  pthread_join(thread, nullptr);
  return 0;
}