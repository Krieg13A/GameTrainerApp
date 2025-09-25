#pragma once
#include <string>

void InitLogger(const std::string& filename);
void LogEvent(const std::string& eventType, const std::string& details);