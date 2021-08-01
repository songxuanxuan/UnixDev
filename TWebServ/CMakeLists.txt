add_executable(tserv "tserv.c" "../TinyWeb/servlib.h" "../TinyWeb/servlib.c" "tservlib.c")

target_link_libraries(tserv pthread)