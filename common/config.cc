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
// 如果` this`的第一个子元素是一个TiXmlText，那么GetText()返回文本节点的字符串，否则返回null
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
    // 读取服务器参数节点
    READ_XML_NODE(server, root_node);

    // 读取日志等级
    READ_STR_FROM_XML_NODE(log_level, log_node);
    READ_STR_FROM_XML_NODE(log_file_name, log_node);
    READ_STR_FROM_XML_NODE(log_file_path, log_node);
    READ_STR_FROM_XML_NODE(log_max_file_size, log_node);
    READ_STR_FROM_XML_NODE(log_sync_interval, log_node);
    READ_STR_FROM_XML_NODE(log_file_app_path, log_node);

    myLogLevel = log_level_str;
    myLogFileName = log_file_name_str;                            // 异步日志器的文件名
    myLogFilePath = log_file_path_str;                            // 异步日志器的文件路径
    myMaxFileSize = std::atoi(log_max_file_size_str.c_str());     // 异步日志器的最大字节数
    myLogSyncInterval = std::atoi(log_sync_interval_str.c_str()); // 日志同步时间间隔，ms
    myAppLogFilePath = log_file_app_path_str;                     // 异步app日志器的文件路径

    printf("LOG -- CONFIG LEVEL[%s], FILE_NAME[%s],FILE_PATH[%s] MAX_FILE_SIZE[%d B], SYNC_INTEVAL[%d ms]\n",
           myLogLevel.c_str(), myLogFileName.c_str(), myLogFilePath.c_str(), myMaxFileSize, myLogSyncInterval);

    READ_STR_FROM_XML_NODE(port, server_node);
    READ_STR_FROM_XML_NODE(io_threads, server_node);

    myPort = std::atoi(port_str.c_str());
    myIOThread = std::atoi(io_threads_str.c_str());
    printf("Server -- PORT[%d], IO Threads[%d]\n", myPort, myIOThread);
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