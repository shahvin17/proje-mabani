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
target_link_libraries(scratch_app  -lconio)

target_include_directories(scratch_app PRIVATE include)
target_link_libraries(scratch_app -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer)
# SDL2
find_package(SDL2 REQUIRED)
target_link_libraries(scratch_app SDL2::SDL2 SDL2::SDL2main)