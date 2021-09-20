add_executable(http "http.cpp" "locker.h" "threadpool.h" "http_conn.h" "http_conn.cpp" "http_time_minheap.h")

target_link_libraries(http pthread)