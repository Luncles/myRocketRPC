#!/bin/bash

if [ -z "$1" ]
then
  echo "Please input rocket server name!"
  exit -1
fi

FILE_NAME=$1
PROJECT_NAME=$(basename ${FILE_NAME})
CURRENT_PATH=$(cd $(dirname $0); pwd)
PROJECT_BIN_FILE="${CURRENT_PATH}"/"${PROJECT_NAME}"
PROJECT_CONF_FILE="../conf/myRocket.xml"

echo "Shutdown rocket rpc project, name: ${PROJECT_NAME}, path: ${PROJECT_BIN_FILE}"

# check bin file exist
if [ ! -e ${PROJECT_BIN_FILE} ]
then
  echo "Shtdown rpcket rpc eror, file: ${PROJECT_BIN_FILE} not exist, please check file"
  exit -1
fi

# 使用ps命令列出当前系统中的所有进程，然后使用grep命令过滤出包含${PROJECT_NAME}的进程。接下来，使用两个grep -v命令排除包含'grep'和'shutdown.sh'的进程。最后，使用awk命令提取出每个进程的PID（进程ID），并将它们存储在一个名为proc_list的数组中。
proc_list=`ps -elf | grep "${PROJECT_NAME}" | grep -v 'grep' | grep -v 'shutdown.sh' | awk '{print $4}'`
echo ${proc_list[0]}

for i in ${proc_list[@]}
do
  bin_path=`ls -l /proc/${i}/exe | awk '{print $11}'`
  echo ${bin_path}
  if [ "$bin_path" = "$PROJECT_BIN_FILE" ]
  then
    echo "kill this proc: ${i}"
    kill -9 ${i}
  fi
done




