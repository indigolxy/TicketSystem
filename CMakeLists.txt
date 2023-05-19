cmake_minimum_required(VERSION 3.15)
project(TicketSystem)

set(CMAKE_CXX_FLAGS "-std=c++17 -Ofast")

add_executable(code src/main.cpp
        src/database/BPlusTree.cpp
        src/TrainSystem.cpp
        src/TicketSystem.cpp
        )