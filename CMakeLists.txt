cmake_minimum_required(VERSION 3.10)
project(ScratchProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(scratch_app
    src/main.cpp
    src/runtime.cpp
    src/persist.cpp
        src/vars_ops.cpp

)

target_include_directories(scratch_app PRIVATE include)