#ifndef COMMON_LOGGER_H
#define COMMON_LOGGER_H

// Standard includes.
#include <string>

// Forward declarations.
class QStatusBar;

//
// Available log levels.
//

enum class LogLevel {
    Debug    = 5,
    Info     = 4,
    Warning  = 3,
    Error    = 2,
    None     = 1
};


//
// Couscous logginc utility class.
//
// It can prints log message in stdout and in
// the gui and if explicitly specified.
//

class Logger
{
  public:
    static void set_level(const LogLevel level);

    static void log_debug(const char* message);
    static void log_info(const char* message);
    static void log_warning(const char* message);
    static void log_error(const char* message);

    static void log_debug(const std::string& message)
    {
        Logger::log_debug(message.c_str());
    }

    static void log_info(const std::string& message)
    {
        Logger::log_info(message.c_str());
    }

    static void log_warning(const std::string& message)
    {
        Logger::log_warning(message.c_str());
    }

    static void log_error(const std::string& message)
    {
        Logger::log_error(message.c_str());
    }

    static void set_gui_bar(QStatusBar* bar);
};

#endif // COMMON_LOGGER_H
