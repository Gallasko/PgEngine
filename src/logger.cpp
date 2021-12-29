#include "logger.h"

std::vector<Logger::LogSinkPtr> Logger::sinks;

std::mutex Logger::_lock;

// Only used for the console sink
#include <iostream>

namespace
{
    std::string logLevelString(const Logger::InfoLevel& level)
    {
        std::string logLevelStringBuffer = "";

        switch (level)
        {
            case Logger::InfoLevel::log:
                logLevelStringBuffer = "[Log]: ";
                break;

            case Logger::InfoLevel::info:
                logLevelStringBuffer = "[Info]: ";
                break;

            case Logger::InfoLevel::alert:
                logLevelStringBuffer = "[Alert]: ";
                break;

            case Logger::InfoLevel::warning:
                logLevelStringBuffer = "[Warning]: ";
                break;

            case Logger::InfoLevel::error:
                logLevelStringBuffer = "[Error]: ";
                break;

            case Logger::InfoLevel::critical:
                logLevelStringBuffer = "[Critical]: ";
                break;
        }

        return logLevelStringBuffer; 
    }

    std::string logPositionString(const std::string& filename, const std::string& objectName, const std::string& function, const int line)
    {
        std::string logPositionStringBuffer = "in " + filename;

        if(!objectName.empty())
           logPositionStringBuffer += " for object: " + objectName;

        if(!function.empty())
            logPositionStringBuffer += " in function: " + function;

        logPositionStringBuffer += ", line " + std::to_string(line);

        return logPositionStringBuffer;
    }
}

void TerminalSink::operator<<(const Logger::Info& log)
{
    std::cout << logLevelString(log.level) << log.scope << ", " << log.message  << logPositionString(log.filename, log.objectName, log.function, log.line) << "\n";
    //if(not ignoreNonErrors and log.level == Logger::InfoLevel::log)
    //    std::cout << log.line << ", " << log.filename << ", " << log.function << ", " << log.objectName << "," << log.scope << ", " << log.message << ", " << static_cast<int>(log.level) << std::endl;
}
