#include "logger.h"

// Only used for the console sink
#include <iostream>

namespace pg
{
    std::vector<Logger::LogSinkPtr> Logger::sinks;

    std::mutex Logger::_lock;

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

        std::string logPositionString(const char* filename, const char* objectName, const char* function, const int line)
        {
            std::string logPositionStringBuffer = " in " + std::string(filename);

            if(objectName)
            logPositionStringBuffer += " for object: " + std::string(objectName);

            if(function)
                logPositionStringBuffer += " in function: " + std::string(function);

            logPositionStringBuffer += ", line " + std::to_string(line);

            return logPositionStringBuffer;
        }
    }

    void Logger::LogSink::operator<<(const Logger::Info& log)
    {
        bool accepted = true;

        // Structure binding can t be done because filter is pointer
        //for (auto [filterName, filter] : filters)
        for (const auto& filter : filters)
            if (filter.second->isFiltered(log))
                accepted = false;

        if(accepted)
            processLog(log);
    }

    void TerminalSink::processLog(const Logger::Info& log)
    {
        std::cout << logLevelString(log.level) << log.scope << ", " << log.message  << logPositionString(log.filename, log.objectName, log.function, log.line) << "\n";
        //if(not ignoreNonErrors and log.level == Logger::InfoLevel::log)
        //    std::cout << log.line << ", " << log.filename << ", " << log.function << ", " << log.objectName << "," << log.scope << ", " << log.message << ", " << static_cast<int>(log.level) << std::endl;
    }

}