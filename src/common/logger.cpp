#include "logger.h"

// Qt includes.
#include <QStatusBar>
#include <QString>

// Standard includes.
#include <iostream>

using namespace std;

namespace
{
    class LoggerImpl
    {
      public:
        static LoggerImpl& get_instance()
        {
            static LoggerImpl instance;
            return instance;
        }

        // The header is only displayed where there is enough room.
        void message(
            const char* header,
            const char* message)
        {
            cout << header << "| " << message << endl;

            if (gui_bar != nullptr)
            {
                gui_bar->showMessage(message);
            }
        }

        LogLevel    level;
        QStatusBar* gui_bar;

      private:
        LoggerImpl()
          : level(LogLevel::Info)
          , gui_bar(nullptr)
        {
        }
    };
}

//
// Couscous logging utility class implementation.
//

void Logger::set_level(const LogLevel level)
{
    LoggerImpl::get_instance().level = level;
}

void Logger::log_debug(const char* message)
{
    if (LoggerImpl::get_instance().level < LogLevel::Debug)
        return;

    LoggerImpl::get_instance().message("debug ", message);
}

void Logger::log_info(const char* message)
{
    if (LoggerImpl::get_instance().level < LogLevel::Info)
        return;

    LoggerImpl::get_instance().message("info  ", message);
}

void Logger::log_error(const char* message)
{
    if (LoggerImpl::get_instance().level < LogLevel::Error)
        return;

    LoggerImpl::get_instance().message("error ", message);
}

void Logger::set_gui_bar(QStatusBar* bar)
{
    LoggerImpl::get_instance().gui_bar = bar;
}

