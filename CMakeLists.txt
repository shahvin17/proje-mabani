cmake_minimum_required(VERSION 3.10)
project(ScratchProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(SDL2 REQUIRED)

# اگر از افزونه‌های دیگر SDL2 استفاده می‌کنید، آنها را نیز اضافه کنید (از کامنت خارج کنید)
# find_package(SDL2_image REQUIRED)
# find_package(SDL2_ttf REQUIRED)
# find_package(SDL2_mixer REQUIRED)

# --- تعریف فایل‌های سورس پروژه ---
# استفاده از یک متغیر برای تمیز نگه داشتن کد
set(APP_SOURCES
    src/main.cpp
    src/runtime.cpp
    src/persist.cpp
    src/render.cpp
    src/control.cpp
    src/vars_ops.cpp
    src/events.cpp
     src/looks.cpp
    src/pen.cpp
)

if(WIN32)
    add_executable(scratch_app WIN32 ${APP_SOURCES})
else()
    add_executable(scratch_app ${APP_SOURCES})
endif()

target_include_directories(scratch_app PRIVATE
    include
    ${SDL2_INCLUDE_DIRS}
    # ${SDL2_IMAGE_INCLUDE_DIRS}
    # ${SDL2_TTF_INCLUDE_DIRS}
    # ${SDL2_MIXER_INCLUDE_DIRS}
)

target_link_libraries(scratch_app
    ${SDL2_LIBRARIES}
    # ${SDL2_IMAGE_LIBRARIES}
    # ${SDL2_TTF_LIBRARIES}
    # ${SDL2_MIXER_LIBRARIES}
)
target_link_libraries(scratch_app -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer)