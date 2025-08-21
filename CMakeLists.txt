cmake_minimum_required(VERSION 4.0)
project(CLIChatServer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(server server/main.cpp)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/common/include)

add_executable(client client/main.cpp)
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/common/include)
