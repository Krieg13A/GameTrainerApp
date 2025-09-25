#include "EventLogger.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

static std::string logFilename = "event_log.csv";

void InitLogger(const std::string& filename) {
    logFilename = filename;
}

std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm;
    localtime_s(&tm, &time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

void LogEvent(const std::string& eventType, const std::string& details) {
    std::ofstream file(logFilename, std::ios::app);
    if (!file.is_open()) return;

    file << GetTimestamp() << ',' << eventType << ',' << details << '\n';
    file.close();
}