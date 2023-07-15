#ifndef MYROCKETRPC_COMMON_CONFIG_H
#define MYROCKETRPC_COMMON_CONFIG_H

#include <string>
#include <tinyxml/tinyxml.h>

namespace myRocket
{
    class Config
    {
    public:
        // 自定义了带参数的构造函数，则编译器不再隐式定义默认构造函数，需显式定义
        Config();
        Config(const char *xmlfile);

        ~Config();

    public:
        // 设置为静态成员函数，以便不用构造对象也能调用
        static Config* GetGlobalConfig();
        static void SetGlobalConfig(const char* xmlfile);

    public:
        TiXmlDocument *myXmlDocument{nullptr}; // 对应xml的整个文档
        std::string myLogLevel;                // 日志等级
    private:
    };
}
#endif