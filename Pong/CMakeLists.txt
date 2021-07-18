cmake_minimum_required(VERSION 3.8)

add_executable(pong "pong.c" "bounce.h")

project("pong")


target_link_libraries(pong ncurses)
target_link_libraries(pong rt)

#add_library(ncurses curses.h)

#find_library(NCURSES curses "/usr/include/")

#link_libraries(ncurses)
#set(CMAKE_EXE_LINKER_FLAGS "-lncurses")

#add_compile_options("-lncurses")

#set(CMAKE_LIBRARY_PATH "/usr/lib/x86_64-linux-gnu/libncurses.so")