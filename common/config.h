#ifndef MYROCKETRPC_COMMON_CONFIG_H
#define MYROCKETRPC_COMMON_CONFIG_H

#include <string>
#include <tinyxml/tinyxml.h>
#include <map>
#include "myRocketRPC/net/tcp/net_addr.h"

namespace myRocketRPC
{
    struct RpcStub
    {
        std::string name;
        IPNetAddr::myNetAddrPtr addr;
        int timeout{2000};
    };

    class Config
    {
    public:
        // 自定义了带参数的构造函数，则编译器不再隐式定义默认构造函数，需显式定义
        Config();
        Config(const char *xmlfile);

        ~Config();

    public:
        // 设置为静态成员函数，以便不用构造对象也能调用
        static Config *GetGlobalConfig();
        static void SetGlobalConfig(const char *xmlfile);

    public:
        TiXmlDocument *myXmlDocument{nullptr}; // 对应xml的整个文档
        std::string myLogLevel;                // 日志等级

        std::string myLogFileName;    // 异步日志器的文件名
        std::string myLogFilePath;    // 异步日志器的文件路径
        std::string myAppLogFilePath; // 异步应用日志器的文件路径
        int myMaxFileSize{0};         // 异步日志器的最大字节数

        int myLogSyncInterval{0}; // 日志同步时间间隔，ms

        int myPort{0};     // 端口号
        int myIOThread{0}; // io线程数

        std::map<std::string, RpcStub> myRpcStubs;

    private:
    };
}
#endif