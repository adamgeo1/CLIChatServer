cmake_minimum_required(VERSION 4.0)
project(CLIChatServer)

set(CMAKE_CXX_STANDARD 23)

add_executable(CLIChatServer server/main.cpp
        client/main.cpp)
