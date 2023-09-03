#设置cmake的最小的版本要求
cmake_minimum_required(VERSION 3.15)
#设置工程名，这里和文件夹同名

project(mymuduo)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../lib)
#编译成动态库的路径
set(LIBRARY_OUTPUT_DIRECTORY ./lib)
#设置调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")


# 定义参与编译的源文件
include_directories (src)
aux_source_directory(src SRC_LIST)

# 编译动态库
add_library(mymuduo SHARED ${SRC_LIST})