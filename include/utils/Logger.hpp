#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace xva {

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
public:
    static Logger& instance();

    void setLevel(LogLevel level) { level_ = level; }

    template<typename... Args>
    void log(LogLevel level, Args&&... args) {
        if (level < level_) return;
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        emit(level, oss.str());
    }

    template<typename... Args>
    void debug  (Args&&... a) { log(LogLevel::DEBUG,   std::forward<Args>(a)...); }
    template<typename... Args>
    void info   (Args&&... a) { log(LogLevel::INFO,    std::forward<Args>(a)...); }
    template<typename... Args>
    void warning(Args&&... a) { log(LogLevel::WARNING, std::forward<Args>(a)...); }
    template<typename... Args>
    void error  (Args&&... a) { log(LogLevel::ERROR,   std::forward<Args>(a)...); }

private:
    Logger() = default;
    LogLevel level_ = LogLevel::INFO;
    void emit(LogLevel level, const std::string& msg);
    const char* levelStr(LogLevel l);
};

#define LOG_DEBUG(...)   xva::Logger::instance().debug(__VA_ARGS__)
#define LOG_INFO(...)    xva::Logger::instance().info(__VA_ARGS__)
#define LOG_WARN(...)    xva::Logger::instance().warning(__VA_ARGS__)
#define LOG_ERROR(...)   xva::Logger::instance().error(__VA_ARGS__)

} // namespace xva
