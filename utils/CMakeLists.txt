# 查找当前目录下的所有源文件
aux_source_directory(. DIR_UTILS_SRCS)
# 生成链接库
add_library (utils ${DIR_UTILS_SRCS})
