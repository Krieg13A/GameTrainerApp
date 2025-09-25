#include "SessionManager.h"
#include <chrono>
#include <iomanip>
#include <sstream>

std::string StartNewSession() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);

    std::ostringstream oss;
    oss << "session_"
        << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S")
        << ".csv";

    return oss.str();
}