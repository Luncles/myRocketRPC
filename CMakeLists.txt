cmake_minimum_required(VERSION 3.5)
project(myRocketRPC)

set(PATH_ROCKET myRocketRPC)
set(PATH_COMM ${PATH_ROCKET}/common)
set(PATH_NET ${PATH_ROCKET}/net)
set(PATH_TCP ${PATH_ROCKET}/net/tcp)
set(PATH_CODER ${PATH_ROCKET}/net/coder)
set(PATH_RPC ${PATH_ROCKET}/net/rpc)

set(PATH_TESTCASES testcases)

set(PATH_INSTALL_LIB_ROOT /usr/lib)
set(PATH_INSTALL_INC_ROOT /usr/include)

set(PATH_INSTALL_INC_COMM ${PATH_INSTALL_INC_ROOT}/${PATH_COMM})
set(PATH_INSTALL_INC_NET ${PATH_INSTALL_INC_ROOT}/${PATH_NET})
set(PATH_INSTALL_INC_TCP ${PATH_INSTALL_INC_ROOT}/${PATH_TCP})
set(PATH_INSTALL_INC_CODER ${PATH_INSTALL_INC_ROOT}/${PATH_CODER})
set(PATH_INSTALL_INC_RPC ${PATH_INSTALL_INC_ROOT}/${PATH_RPC})

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 11)
# -g:表示生成调试信息。 -O0 选项表示不进行优化。-Wall 选项表示开启所有警告信息。-Wno-deprecated 选项表示禁用关于废弃特性的警告信息。
# -Wno-unused-but-set-variable 选项表示禁用关于未使用但已设置变量的警告信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wno-deprecated -Wno-unused-but-set-variable")

# 添加包含目录
include_directories(${PATH_ROCKET} ${PATH_COMM} ${PATH_NET} ${PATH_TCP} ${PATH_CODER} ${PATH_RPC})

# 查找库文件
target_link_libraries(myRocket protobuf tinyxml)

# 查找目标文件路径
file(GLOB COMM_SRC ${PATH_COMM}/*.cc)
file(GLOB NET_SRC ${PATH_NET}/*.cc)
file(GLOB TCP_SRC ${PATH_TCP}/*.cc)
file(GLOB CODER_SRC ${PATH_CODER}/*.cc)
file(GLOB RPC_SRC ${PATH_RPC}/*.cc)

add_library(myRocketRPC STATIC ${COMM_SRC} ${NET_SRC} ${TCP_SRC} ${CODER_SRC} ${RPC_SRC})

set(TEST_CASES test_log test_eventloop test_tcp test_client test_rpc_server test_rpc_client)

foreach(test_case IN LISTS TEST_CASES)
    # 定义可执行目标
    add_executable(${test_case} testcases/${test_case}.cc)
    target_link_libraries(${test_case} myRocketRPC dl pthread)
endforeach()

# ... add the previous CMake code here ...

# 定义清理操作
add_custom_target(clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${PATH_OBJ}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${PATH_LIB}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${PATH_BIN}
)

# 定义安装操作
install(TARGETS myRocketRPC ARCHIVE DESTINATION ${PATH_INSTALL_LIB_ROOT})
install(DIRECTORY ${PATH_COMM} DESTINATION ${PATH_INSTALL_INC_ROOT} FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${PATH_NET} DESTINATION ${PATH_INSTALL_INC_ROOT} FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${PATH_TCP} DESTINATION ${PATH_INSTALL_INC_ROOT} FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${PATH_CODER} DESTINATION ${PATH_INSTALL_INC_ROOT} FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ${PATH_RPC} DESTINATION ${PATH_INSTALL_INC_ROOT} FILES_MATCHING PATTERN "*.h")

# 定义卸载操作
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PATH_INSTALL_INC_ROOT}/rocket
    COMMAND ${CMAKE_COMMAND} -E remove ${PATH_INSTALL_LIB_ROOT}/librocket.a
)
