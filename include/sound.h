#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <vector>

// ── ساختار یک صدا ────────────────────────────────────────────────────────────
struct SoundEntry {
    std::string name;
    std::string path;      // مسیر فایل WAV/MP3
};

// ── SoundManager ─────────────────────────────────────────────────────────────
struct SoundManager {
    std::vector<SoundEntry> sounds;
    bool initialized = false;
    float volume     = 1.0f;   // 0.0 - 1.0
};

// ── API ──────────────────────────────────────────────────────────────────────
bool sound_init(SoundManager& sm);
void sound_quit(SoundManager& sm);

void sound_add(SoundManager& sm, const std::string& name, const std::string& path);
void sound_play(SoundManager& sm, const std::string& name);
void sound_play_index(SoundManager& sm, int index);
void sound_stop_all(SoundManager& sm);
void sound_set_volume(SoundManager& sm, float vol);   // 0.0 - 1.0

// backward compat (بدون SoundManager)
void sound_play(const std::string& name);
void sound_stop_all();

#endif