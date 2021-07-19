# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
cmake_minimum_required (VERSION 3.8)

project ("UnixDev")

# 包含子项目。
add_subdirectory ("UnixDev")
add_subdirectory ("Mymore")
add_subdirectory ("MyWho")
add_subdirectory ("MyCp")
add_subdirectory ("MyTail")
add_subdirectory ("MyLs")
add_subdirectory ("MyPwd")
add_subdirectory ("MyMv")
add_subdirectory ("MyWrite")
add_subdirectory ("Pong")
add_subdirectory ("MySh")
add_subdirectory ("Test")
