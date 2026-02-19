cmake_minimum_required(VERSION 3.10)
project(ScratchProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(scratch_app
    src/main.cpp
    src/runtime.cpp
    src/persist.cpp
    src/vars_ops.cpp
    src/events.cpp
    src/control.cpp
)

target_include_directories(scratch_app PRIVATE include)

# SDL2
find_package(SDL2 REQUIRED)
target_link_libraries(scratch_app SDL2::SDL2 SDL2::SDL2main)