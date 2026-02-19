cmake_minimum_required(VERSION 3.10)
project(ScratchProject)

cmake_minimum_required(VERSION 3.10)
project(ScratchProject)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

add_executable(scratch_app
    src/main.cpp
    src/render.cpp
    src/runtime.cpp
    src/persist.cpp
    src/vars_ops.cpp
    src/control.cpp
)

target_include_directories(scratch_app PRIVATE include)

target_link_libraries(scratch_app SDL2::SDL2 SDL2::SDL2main)