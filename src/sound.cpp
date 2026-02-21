#include "sound.h"
#include <iostream>

// در این نسخه sound فقط stub است
// برای پیاده‌سازی کامل از SDL2_mixer استفاده کنید

void sound_play(const std::string& name) {
    std::cout << "[Sound] play: " << name << std::endl;
}

void sound_stop_all() {
    std::cout << "[Sound] stop all" << std::endl;
}