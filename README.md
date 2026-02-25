# Scratch C++ — پیاده‌سازی Scratch در C++ با SDL2

> پروژه درس مبانی برنامه‌نویسی، دانشگاه صنعتی شریف  
> یک محیط برنامه‌نویسی بصری شبیه Scratch، پیاده‌سازی‌شده با C++17 و SDL2

---

## 🗂️ ساختار پروژه

```
ScratchCpp/
├── src/
│   ├── main.cpp          ← رابط کاربری، event loop، رندر اصلی
│   ├── runtime.cpp/h     ← موتور اجرای بلوک‌ها (per-sprite, clone, broadcast)
│   ├── core_types.h      ← ساختارهای اصلی (Block, Sprite, Project, ...)
│   ├── motion.cpp/h      ← دستورات حرکتی
│   ├── looks.cpp/h       ← دستورات ظاهری
│   ├── pen.cpp/h         ← سیستم قلم (Pen extension)
│   ├── sensing.cpp/h     ← سیستم حسگری (ماوس، کیبورد، تایمر)
│   ├── sound.cpp/h       ← سیستم صدا (SDL_mixer)
│   ├── vars_ops.cpp/h    ← متغیرها و عملگرها
│   ├── persist.cpp/h     ← ذخیره/بارگذاری پروژه
│   └── logger.cpp/h      ← سیستم لاگ
├── include/              ← header های مشترک
├── assets/
│   ├── fonts/            ← DejaVu Sans (TTF)
│   ├── sounds/           ← فایل‌های WAV پیش‌فرض
│   └── images/           ← sprite و backdrop پیش‌فرض
├── CMakeLists.txt
└── README.md
```

---

## 🔧 نیازمندی‌ها

| ابزار | نسخه |
|-------|-------|
| C++ Compiler | GCC 9+ / MSVC 2019+ / Clang 10+ |
| CMake | 3.16+ |
| SDL2 | 2.0.14+ |
| SDL2_ttf | 2.0.15+ |
| SDL2_mixer (اختیاری) | 2.0.4+ |

---

## 🚀 ساخت روی ویندوز

### روش ۱ — vcpkg (توصیه‌شده)

```powershell
# نصب vcpkg
git clone https://github.com/microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat

# نصب کتابخانه‌ها
.\vcpkg install sdl2 sdl2-ttf sdl2-mixer --triplet x64-windows

# Build پروژه
cd C:\path\to\ScratchCpp
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

# اجرا
.\Release\ScratchCpp.exe
```

### روش ۲ — SDL2 دستی

۱. دانلود SDL2, SDL2_ttf از https://libsdl.org  
۲. اضافه کردن مسیر include و lib به پروژه  
۳. کپی DLL ها کنار exe

---

## 🐧 ساخت روی Linux

```bash
# نصب وابستگی‌ها
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev cmake g++

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./ScratchCpp
```

---

## 🍎 ساخت روی macOS

```bash
brew install sdl2 sdl2_ttf sdl2_mixer cmake

mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
./ScratchCpp
```

---

## 🎮 راهنمای استفاده

### رابط کاربری

```
┌─────────────────────────────────────────────────────────┐
│  TOOLBAR:  ▶Run  ■Stop  ||Pause  ↩Z  ↪Y  ⚡Turbo  ▶|Step│
├──────────┬──────────────────────────┬────────────────────┤
│          │                          │                    │
│ PALETTE  │    SCRIPT CANVAS         │     STAGE          │
│ (بلوک‌ها) │  (drag & drop بلوک‌ها)   │  (اجرای برنامه)   │
│          │                          │                    │
├──────────┴──────────────────────────┴────────────────────┤
│  SPRITE PANEL:  [+Add]  [Sprite1] [Sprite2] ...          │
└─────────────────────────────────────────────────────────┘
```

### کنترل‌های کیبورد

| کلید | عملکرد |
|------|--------|
| `F5` | اجرای برنامه |
| `Escape` | توقف / بستن panel |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Ctrl+S` | ذخیره پروژه |
| `Delete` | حذف بلوک انتخابی |
| `Space/Arrow` | event های when_key_pressed |

### درگ و دراپ بلوک‌ها

1. **کلیک روی بلوک در palette** → کپی به canvas
2. **درگ بلوک در canvas** → جابجایی
3. **نزدیک کردن پایین یک بلوک به بالای دیگری** → snap اتوماتیک
4. **راست‌کلیک روی بلوک** → منوی Duplicate / Delete / Comment / Disable

---

## 🧩 بلوک‌های پیاده‌سازی‌شده

### Motion (آبی)
`move`, `turn right/left`, `goto xy`, `set x/y`, `change x/y`, `bounce`, `goto mouse`

### Looks (بنفش)
`say`, `say for secs`, `think`, `show`, `hide`, `set size`, `change size`, `next/prev costume`, `set costume`, `next/prev backdrop`, `set backdrop`

### Sound (صوتی)
`play sound`, `play sound until done`, `stop all sounds`, `set volume`, `change volume`

### Events (زرد)
`when green flag clicked`, `when key pressed`, `when sprite clicked`, `broadcast`, `broadcast and wait`, `when I receive`

### Control (نارنجی)
`wait`, `repeat`, `forever`, `if then`, `if else`, `stop all`, `stop this script`, `create clone`, `when I start as a clone`, `delete this clone`

### Sensing (آبی روشن)
`touching mouse`, `ask and wait`, `mouse x/y`, `key pressed`, `reset timer`

### Operators (سبز)
عملگرهای ریاضی و مقایسه‌ای

### Variables (نارنجی تیره)
`set var`, `change var`, `show/hide var`, `list add/delete/insert/replace`

### Pen (سبز تیره — Extension)
`pen down/up`, `erase all`, `stamp`, `set color`, `set size`, `set hue/brightness/saturation`

---

## 🐛 راهنمای Debug

### Step-by-step Debugger
1. کلیک دکمه `▶|` در toolbar → step mode فعال
2. هر کلیک بعدی → یک بلوک اجرا می‌شود
3. بلوک در حال اجرا با رنگ **سبز چشمک‌زن** highlight می‌شود
4. `ESC` → خروج از step mode

### سیستم Log
تمام رویدادها در console چاپ می‌شوند:
```
[Runtime] Sprite 1 started at block 101
[Broadcast] message1
[Clone] Created clone of sprite 1
[Ask] Question: What's your name?
```

### Undo/Redo
- تا ۵۰ action قابل undo
- عملیات: drag بلوک، snap، تغییر مقدار input

---

## 📁 فرمت فایل پروژه (.fop)

فایل پروژه به صورت JSON ذخیره می‌شود:
```json
{
  "sprites": [...],
  "blocks": [...],
  "variables": [...],
  "backdrops": [...]
}
```
---

## 👥 تیم

پروژه درس مبانی برنامه‌نویسی — دانشگاه صنعتی شریف — 1404