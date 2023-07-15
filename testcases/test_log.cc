#include <pthread.h>
#include "/root/Code/myRocketRPC/common/log.h"
#include "/root/Code/myRocketRPC/common/config.h"

void *PthreadFunc(void *)
{
    DEBUGLOG("new thread test debug log %s", "11");
    INFOLOG("new thread test info log %s", "22");
    return nullptr;
}

int main()
{
    myRocket::Config::SetGlobalConfig("/root/Code/myRocketRPC/conf/myRocket.xml");
    myRocket::Logger::InitGlobalLogger(1);

    pthread_t thread;
    pthread_create(&thread, nullptr, &PthreadFunc, nullptr);

    DEBUGLOG("test debug log %s", "11");
    INFOLOG("test info log %s", "22");

    pthread_join(thread, nullptr);
    return 0;
}