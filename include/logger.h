
#ifndef LOGGER_H
#define LOGGER_H
#include <iostream>
using namespace std;
#include <fstream>
#include <string>
void logInfo(const std::string& msg);
void logWarning(const std::string& msg);
void logError(const std::string& msg);
#include "core_types.h"
#include <string>

std::string get_system_help_text();

void safety_clamp_sprite(Sprite& spr, float screen_width, float screen_height);

struct WatchdogState {
    int current_frame_instructions = 0;
    int max_allowed_instructions = 5000;
};

void safety_reset_watchdog(WatchdogState& wd);

bool safety_check_watchdog(WatchdogState& wd, std::string& out_error_message);
#endif

