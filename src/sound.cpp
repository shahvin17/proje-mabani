#include "sound.h"
#include <iostream>
#include <algorithm>

// ──────────────────────────────────────────────────────────────────────────────
// SDL_mixer integration
// اگر SDL_mixer نصب باشه، صدا واقعی پخش می‌شه
// اگر نباشه، stub (فقط log) کار می‌کنه
// ──────────────────────────────────────────────────────────────────────────────

#ifdef USE_SDL_MIXER
  #include <SDL2/SDL_mixer.h>
  static std::vector<Mix_Chunk*> g_chunks;
  static bool g_mixer_ok = false;
#endif

bool sound_init(SoundManager& sm) {
#ifdef USE_SDL_MIXER
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "[Sound] Mix_OpenAudio failed: " << Mix_GetError() << "\n";
        sm.initialized = false;
        return false;
    }
    Mix_AllocateChannels(16);
    g_mixer_ok = true;
    std::cout << "[Sound] SDL_mixer initialized\n";
#else
    std::cout << "[Sound] Running in stub mode (no SDL_mixer)\n";
#endif
    sm.initialized = true;
    return true;
}

void sound_quit(SoundManager& sm) {
#ifdef USE_SDL_MIXER
    for (auto* c : g_chunks) if (c) Mix_FreeChunk(c);
    g_chunks.clear();
    if (g_mixer_ok) { Mix_CloseAudio(); g_mixer_ok = false; }
#endif
    sm.initialized = false;
}

void sound_add(SoundManager& sm, const std::string& name, const std::string& path) {
    // بررسی تکراری نبودن
    for (auto& s : sm.sounds) if (s.name == name) return;
    SoundEntry e; e.name = name; e.path = path;
    sm.sounds.push_back(e);
#ifdef USE_SDL_MIXER
    if (g_mixer_ok) {
        Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
        g_chunks.push_back(chunk);  // index موازی با sm.sounds
        if (!chunk)
            std::cerr << "[Sound] Load failed: " << path << " - " << Mix_GetError() << "\n";
        else
            std::cout << "[Sound] Loaded: " << name << "\n";
    } else {
        g_chunks.push_back(nullptr);
    }
#endif
    std::cout << "[Sound] Added: " << name << " (" << path << ")\n";
}

void sound_play(SoundManager& sm, const std::string& name) {
    for (size_t i = 0; i < sm.sounds.size(); ++i) {
        if (sm.sounds[i].name == name) {
            sound_play_index(sm, (int)i);
            return;
        }
    }
    std::cout << "[Sound] Not found: " << name << "\n";
}

void sound_play_index(SoundManager& sm, int index) {
    if (index < 0 || index >= (int)sm.sounds.size()) return;
    std::cout << "[Sound] Play: " << sm.sounds[index].name << "\n";
#ifdef USE_SDL_MIXER
    if (g_mixer_ok && index < (int)g_chunks.size() && g_chunks[index]) {
        int vol = (int)(sm.volume * MIX_MAX_VOLUME);
        Mix_VolumeChunk(g_chunks[index], vol);
        Mix_PlayChannel(-1, g_chunks[index], 0);
    }
#endif
}

void sound_stop_all(SoundManager& sm) {
    std::cout << "[Sound] Stop all\n";
#ifdef USE_SDL_MIXER
    if (g_mixer_ok) Mix_HaltChannel(-1);
#endif
}

void sound_set_volume(SoundManager& sm, float vol) {
    sm.volume = std::max(0.f, std::min(1.f, vol));
#ifdef USE_SDL_MIXER
    if (g_mixer_ok) Mix_Volume(-1, (int)(sm.volume * MIX_MAX_VOLUME));
#endif
}

// ── backward compat ───────────────────────────────────────────────────────────
void sound_play(const std::string& name) {
    std::cout << "[Sound] play: " << name << "\n";
}
void sound_stop_all() {
    std::cout << "[Sound] stop all\n";
}