#!/bin/bash
FILE_NAME=$1
PROJECT_NAME=$(basename ${FILE_NAME})
CURRENT_PATH=$(cd $(dirname $0); pwd)
PROJECT_ROOT_PATH=$(cd $(dirname $0); cd ..; pwd)
PROJECT_BIN_FILE="${CURRENT_PATH}"/"${PROJECT_NAME}"
PROJECT_CONF_FILE="../conf/rocket.xml"


echo "Run rocket rpc project, name: ${PROJECT_NAME}, path: ${PROJECT_BIN_FILE}"

# -z：是否为空
if [ -z "$1" ]
then
  echo "Please input execuable binary file!"
fi

# check bin file exist
if [ ! -e ${PROJECT_BIN_FILE} ]
then
  echo "Run rocket rpc server eror, file: ${PROJECT_BIN_FILE} not exist, please check file"
  exit -1
fi

# check config xml file exist
if [ ! -e ${PROJECT_CONF_FILE} ]
then
  echo "Run rocket rpc error, file: ${PROJECT_CONF_FILE} not exist, please check config file"
  exit -1
fi

# check bin file execute privilege  -x：可执行权限
if [ ! -x ${PROJECT_BIN_FILE} ]
then
  echo "chmod +x : ${PROJECT_BIN_FILE}"
  chmod +x ${PROJECT_BIN_FILE}
fi

bash shutdown.sh ${PROJECT_NAME}
# nohup & 表示忽略终端的SIGHUP信号，也就是在后台执行 >:表示重定向
nohup ./${PROJECT_NAME} ${PROJECT_CONF_FILE} & > ${PROJECT_ROOT_PATH}/log/${PROJECT_NAME}.nohup_log
echo "Start rocket rpc server ${PROJECT_CONF_FILE} succ"




