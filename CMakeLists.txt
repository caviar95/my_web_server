cmake_minimum_required(VERSION 3.28)

project(web_server)

set(CMAKE_CXX_STANDARD 17)

# add_subdirectory(src)

add_executable(web_server
    # ${MY_SRC_FILES}

    src/main.cpp
    src/request_handler.cpp
    src/server.cpp
    src/thread_pool.cpp
)

target_include_directories(web_server PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(web_server pthread)
