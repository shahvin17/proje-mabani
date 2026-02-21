#include <logger.h>
#include <algorithm>

using namespace std;

string get_system_help_text() {
    return
    "=== Scratch C++ System Help ===\n\n"
    "1. Debug Mode (Step-by-Step):\n"
    "   - Enable Debug Mode from the menu to pause execution.\n"
    "   - Press 'Space' to advance the program one block at a time.\n\n"
    "2. System Logger (Black Box):\n"
    "   - The system records every executed command (Cycle, Line, CMD).\n"
    "   - Check the console or log file for execution history.\n\n"
    "3. Safety Nets:\n"
    "   - Boundary Check: Sprites cannot move outside the screen.\n"
    "   - Math Safeguard: Division by zero and sqrt(negative) are prevented.\n"
    "   - Infinite Loop Watchdog: Halts scripts stuck in a 'Forever' loop.\n"
    "===============================\n";
}

void safety_clamp_sprite(Sprite& spr, float screen_width, float screen_height) {

    float half_w = (spr.width > 0) ? spr.width / 2.0f : 25.0f;
    float half_h = (spr.height > 0) ? spr.height / 2.0f : 25.0f;


    spr.x = std::max(half_w, std::min(spr.x, screen_width - half_w));

    spr.y = std::max(half_h, std::min(spr.y, screen_height - half_h));
}

void safety_reset_watchdog(WatchdogState& wd) {
    wd.current_frame_instructions = 0;
}

bool safety_check_watchdog(WatchdogState& wd, string& out_error_message) {
    wd.current_frame_instructions++;


    if (wd.current_frame_instructions > wd.max_allowed_instructions) {

        out_error_message = "[Watchdog Error] Infinite loop detected! Script halted.";
        return true;
    }

    return false;
}
