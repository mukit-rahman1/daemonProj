cmake_minimum_required(VERSION 3.16)
project(rtjitter CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rtmonitord src/rtmonitord.cpp)
target_include_directories(rtmonitord PRIVATE include)
target_compile_options(rtmonitord PRIVATE -Wall -Wextra -Wpedantic -O2)

add_executable(rtctl src/rtctl.cpp)
target_include_directories(rtctl PRIVATE include)
target_compile_options(rtctl PRIVATE -Wall -Wextra -Wpedantic -O2)