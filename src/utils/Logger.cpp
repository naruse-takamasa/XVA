#include "utils/Logger.hpp"

#include <chrono>
#include <ctime>
#include <iostream>
#include <string>

namespace xva {

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

const char* Logger::levelStr(LogLevel l) {
    switch (l) {
        case LogLevel::DBG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO ";
        case LogLevel::WARNING:
            return "WARN ";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "?????";
    }
}

void Logger::emit(LogLevel level, const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[20];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    std::cout << "[" << buf << "][" << levelStr(level) << "] " << msg << "\n";
}

}  // namespace xva
