
#ifndef LOGGER_H
#define LOGGER_H
#include <iostream>
using namespace std;
#include <fstream>
#include <string>
void logInfo(const std::string& msg);
void logWarning(const std::string& msg);
void logError(const std::string& msg);
#endif

