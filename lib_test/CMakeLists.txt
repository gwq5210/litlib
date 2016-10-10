# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)
# 项目信息
project (lib_test)

# 添加子目录
add_subdirectory(../utils)
#使用 aux_source_directory 命令，该命令会查找指定目录下的所有源文件，然后将结果存进指定变量名
aux_source_directory(. CURR_SRCS)
# 指定生成目标
add_executable(lib_test ${CURR_SRCS})

#添加链接库
target_link_libraries(lib_test utils)
