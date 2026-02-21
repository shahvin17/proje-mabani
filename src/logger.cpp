#include "logger.h"
#include <iostream>
#include <chrono>
#include <ctime>

static std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));
    return std::string(buf);
}

void logInfo(const std::string& msg) {
    std::cout << "[INFO]  " << timestamp() << " | " << msg << std::endl;
}
void logWarning(const std::string& msg) {
    std::cout << "[WARN]  " << timestamp() << " | " << msg << std::endl;
}
void logError(const std::string& msg) {
    std::cerr << "[ERROR] " << timestamp() << " | " << msg << std::endl;
}