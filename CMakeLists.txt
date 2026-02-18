cmake_minimum_required(VERSION 3.20)
project(proje_mabani LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(proje_mabani
        src/main.cpp
        src/persist.cpp
        src/logger.cpp
        src/events.cpp
        src/control.cpp
        src/looks.cpp
        src/motion.cpp
        src/pen.cpp
        src/render.cpp
        src/runtime.cpp
        src/sensing.cpp
        src/sound.cpp
        src/ui_drag.cpp
        src/vars_ops.cpp
)

target_include_directories(proje_mabani PRIVATE include)