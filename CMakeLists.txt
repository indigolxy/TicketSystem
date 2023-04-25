cmake_minimum_required(VERSION 3.15)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 14)

add_executable(code src/bpt/main.cpp
        src/bpt/bpt.cpp)

add_executable(makedata src/test/bpt_makedata.cpp)

add_executable(test src/bpt/submit_file.cpp)

add_executable(std_test src/test/bpt_std.cpp)