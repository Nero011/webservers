#设置cmake的最小的版本要求
cmake_minimum_required(VERSION 3.15)
#设置工程名，这里和文件夹同名
project(test)
#使用include_directories命令来添加包含的头文件目录
include_directories(src)
#使用set来设置一个变量SOURCES，变量的值为空格后面的值
#set(SOURCES src/main.cpp src/Student.cpp)

#这里也是设置一个变量SOURCES，和上面set的效果一样
file(GLOB SOURCES "src/*.cc")
#添加可执行文件，第二个参数这里使用的是变量，相当于src/main.cpp src/Student.cpp
add_executable(threadPoolTest ${SOURCES})