cmake_minimum_required(VERSION 3.15)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 14)

add_executable(code src/database/main.cpp
        src/database/BPlusTree.cpp
        src/user/UserSystem.cpp
        )

add_executable(makedata src/test/bpt_makedata.cpp)

add_executable(std_test src/test/bpt_std.cpp)