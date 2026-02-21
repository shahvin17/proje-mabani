cmake_minimum_required(VERSION 3.10)
project(ScratchProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# بخش ۱: تعریف صریح و مستقیم تمام فایل‌های سورس
# دیگر از GLOB استفاده نمی‌کنیم تا هیچ ابهامی وجود نداشته باشد.
set(APP_SOURCES
    src/main.cpp
    src/control.cpp
    src/events.cpp
    src/logger.cpp
    src/looks.cpp
    src/motion.cpp
    src/pen.cpp
    src/persist.cpp
    src/render.cpp
    src/runtime.cpp
    src/sensing.cpp
    src/sound.cpp
    src/ui_drag.cpp
    src/vars_ops.cpp
)

# بخش ۲: معرفی پوشه هدرها به کامپایلر
# از متغیر عمومی include_directories استفاده می‌کنیم که بسیار مستقیم عمل می‌کند.
include_directories(include)

# بخش ۳: ساخت فایل اجرایی
add_executable(scratch_app ${APP_SOURCES})

# بخش ۴: لینک کردن صریح و مستقیم کتابخانه‌ها (مهم‌ترین بخش)
# به جای استفاده از متغیرها، نام کتابخانه‌ها را مستقیماً به لینکر می‌دهیم.
# پیش‌نیاز: باید پکیج‌های libsdl2-dev و libsdl2-ttf-dev در سیستم شما نصب شده باشند.
target_link_libraries(scratch_app
    SDL2
    SDL2_ttf
)
