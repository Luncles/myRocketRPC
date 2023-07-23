#include "config.h"

#include <tinyxml/tinyxml.h>

// #name意思是展开为字符串 ##是联结两个变量
// 读取一个xml节点
#define READ_XML_NODE(name, parent)                                           \
  TiXmlElement *name##_node = parent->FirstChildElement(#name);               \
  if (!name##_node)                                                           \
  {                                                                           \
    printf("Start myRocket server error, failed to read node [%s]\n", #name); \
    exit(0);                                                                  \
  }

// 读取节点的string
// 访问元素中的文本。虽然简单简洁，与直接取得TiXmlText子元素并访问它相比，GetText()有局限性。
// 如果` this
// `的第一个子元素是一个TiXmlText，那么GetText()返回文本节点的字符串，否则返回null
#define READ_STR_FROM_XML_NODE(name, parent)                                 \
  TiXmlElement *name##_node = parent->FirstChildElement(#name);              \
  if (!name##_node || !name##_node->GetText())                               \
  {                                                                          \
    printf("Start myRocket server error, failed to read config file [%s]\n", \
           #name);                                                           \
    exit(0);                                                                 \
  }                                                                          \
  std::string name##_str = std::string(name##_node->GetText());

namespace myRocket
{
  // 设置为static，意味着这个指针属于类，而不属于类的任何一个实例，这个指针在整个程序运行期间都存在，且只有一个
  // 单例模式
  static Config *globalConfig = nullptr;

  Config *Config::GetGlobalConfig() { return globalConfig; }

  // 单例模式
  void Config::SetGlobalConfig(const char *xmlfile)
  {
    if (globalConfig == nullptr)
    {
      if (xmlfile != nullptr)
      {
        globalConfig = new Config(xmlfile);
      }
      else
      {
        globalConfig = new Config();
      }
    }
  }

  // 默认构造函数
  Config::Config() { myLogLevel = "DEBUG"; }

  Config::Config(const char *xmlfile)
  {
    myXmlDocument = new TiXmlDocument();

    bool loadFile = myXmlDocument->LoadFile(xmlfile);

    if (!loadFile)
    {
      printf(
          "Start myRocket server error, failed to read config file %s, error "
          "info[%s]\n",
          xmlfile, myXmlDocument->ErrorDesc());
      exit(0);
    }

    // 读取根节点
    READ_XML_NODE(root, myXmlDocument);
    // 读取日志节点
    READ_XML_NODE(log, root_node);
    // 读取

    // 读取日志等级
    READ_STR_FROM_XML_NODE(log_level, log_node);

    myLogLevel = log_level_str;
  }

  // 析构函数
  Config::~Config()
  {
    if (myXmlDocument)
    {
      delete myXmlDocument;
      myXmlDocument = nullptr;
    }
  }

} // namespace myRocket