cmake_minimum_required(VERSION 3.10)
project(ScratchProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- یافتن وابستگی‌ها (Dependencies) ---
# این روش مدرن و کراس-پلتفرم برای پیدا کردن SDL2 است.
# CMake به طور خودکار در مسیرهای استاندارد سیستم به دنبال آن می‌گردد.
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
    # هر فایل cpp جدیدی که ساختید، اینجا اضافه کنید
    # src/looks.cpp
    # src/pen.cpp
)

# --- تعریف فایل اجرایی ---
# پرچم WIN32 برای ویندوز مهم است تا برنامه بدون باز شدن یک پنجره کنسول اضافی اجرا شود.
# این دستور به صورت شرطی فقط روی ویندوز اعمال می‌شود.
if(WIN32)
    add_executable(scratch_app WIN32 ${APP_SOURCES})
else()
    add_executable(scratch_app ${APP_SOURCES})
endif()

# --- تنظیم مسیرهای Header ---
target_include_directories(scratch_app PRIVATE
    include                  # مسیر هدرهای پروژه شما
    ${SDL2_INCLUDE_DIRS}     # مسیر هدرهای SDL2 که به صورت خودکار پیدا شده
    # ${SDL2_IMAGE_INCLUDE_DIRS} # برای افزونه‌ها
    # ${SDL2_TTF_INCLUDE_DIRS}
    # ${SDL2_MIXER_INCLUDE_DIRS}
)

# --- لینک کردن کتابخانه‌ها ---
target_link_libraries(scratch_app
    ${SDL2_LIBRARIES}
    # ${SDL2_IMAGE_LIBRARIES}
    # ${SDL2_TTF_LIBRARIES}
    # ${SDL2_MIXER_LIBRARIES}
)