cmake_minimum_required(VERSION 3.10)
project(ScratchProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── یافتن SDL2 ────────────────────────────────────────────────────────────────
find_package(SDL2 REQUIRED)

# SDL2_ttf — برای نمایش متن روی بلوک‌ها
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SDL2_TTF QUIET SDL2_ttf)
endif()

if(NOT SDL2_TTF_FOUND)
    find_library(SDL2_TTF_LIB NAMES SDL2_ttf libSDL2_ttf)
    find_path(SDL2_TTF_INC  NAMES SDL_ttf.h PATH_SUFFIXES SDL2)
    if(SDL2_TTF_LIB AND SDL2_TTF_INC)
        set(SDL2_TTF_LIBRARIES    ${SDL2_TTF_LIB})
        set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INC})
        set(SDL2_TTF_FOUND TRUE)
        message(STATUS "SDL2_ttf found: ${SDL2_TTF_LIB}")
    else()
        message(WARNING "SDL2_ttf NOT found — install: sudo apt install libsdl2-ttf-dev")
        set(SDL2_TTF_LIBRARIES    "")
        set(SDL2_TTF_INCLUDE_DIRS "")
    endif()
endif()

# ── فایل‌های سورس ─────────────────────────────────────────────────────────────
set(APP_SOURCES
    src/main.cpp
    src/runtime.cpp
    src/persist.cpp
    src/render.cpp
    src/control.cpp
    src/vars_ops.cpp
    src/events.cpp
    src/looks.cpp
    src/motion.cpp
    src/pen.cpp
    src/sensing.cpp
    src/logger.cpp
    src/sound.cpp
    src/ui_drag.cpp
)

# ── Executable ────────────────────────────────────────────────────────────────
if(WIN32)
    add_executable(scratch_app WIN32 ${APP_SOURCES})
else()
    add_executable(scratch_app ${APP_SOURCES})
endif()

# ── Includes ──────────────────────────────────────────────────────────────────
target_include_directories(scratch_app PRIVATE
    include
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

# ── Link ──────────────────────────────────────────────────────────────────────
if(WIN32)
    target_link_libraries(scratch_app
        -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
    )
else()
    target_link_libraries(scratch_app
        ${SDL2_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
    )
endif()

message(STATUS "SDL2 libs:    ${SDL2_LIBRARIES}")
message(STATUS "SDL2_ttf:     ${SDL2_TTF_LIBRARIES}")