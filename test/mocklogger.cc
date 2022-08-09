#include "mocklogger.h"

#include <iostream>

namespace pg
{
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

    TestSink::TestSink(bool verbose) : Logger::LogSink(), verbose(verbose)
    {
        resetSink();
    }

    void TestSink::resetSink()
    {
        nbMessages[Logger::InfoLevel::log]      = 0;
        nbMessages[Logger::InfoLevel::info]     = 0;
        nbMessages[Logger::InfoLevel::alert]    = 0;
        nbMessages[Logger::InfoLevel::warning]  = 0;
        nbMessages[Logger::InfoLevel::error]    = 0;
        nbMessages[Logger::InfoLevel::critical] = 0;

        lastMessage = {"", Logger::InfoLevel::log};
    }

    void TestSink::processLog(const Logger::Info& log)
    {
        if(verbose)
            std::cout << logLevelString(log.level) << log.scope << " " << log.message  << logPositionString(log.filename, log.objectName, log.function, log.line) << "\n";

        nbMessages[log.level]++;

        lastMessage = {log.message, log.level};
    }

    MockLogger::MockLogger(bool verbose)
    {
        sink = Logger::registerSink<TestSink>(verbose);
    }

    MockLogger::~MockLogger()
    {
        Logger::removeSink(sink);
    }

    void MockLogger::reset()
    {
        auto pointer = std::static_pointer_cast<TestSink>(sink);

        pointer->resetSink();
    }

    unsigned int MockLogger::getNbLog() const
    {
        auto pointer = std::static_pointer_cast<TestSink>(sink);

        return pointer->getNbMessages().at(Logger::InfoLevel::log);
    }

    unsigned int MockLogger::getNbInfo() const
    {
        auto pointer = std::static_pointer_cast<TestSink>(sink);

        return pointer->getNbMessages().at(Logger::InfoLevel::info);
    }

    unsigned int MockLogger::getNbAlert() const
    {
        auto pointer = std::static_pointer_cast<TestSink>(sink);

        return pointer->getNbMessages().at(Logger::InfoLevel::alert);
    }

    unsigned int MockLogger::getNbWarning() const
    {
        auto pointer = std::static_pointer_cast<TestSink>(sink);

        return pointer->getNbMessages().at(Logger::InfoLevel::warning);
    }

    unsigned int MockLogger::getNbError() const
    {
        auto pointer = std::static_pointer_cast<TestSink>(sink);

        return pointer->getNbMessages().at(Logger::InfoLevel::error);
    }

    unsigned int MockLogger::getNbCritical() const
    {
        auto pointer = std::static_pointer_cast<TestSink>(sink);

        return pointer->getNbMessages().at(Logger::InfoLevel::critical);
    }

    const LogMessage& MockLogger::getLastMessage() const
    {
        auto pointer = std::static_pointer_cast<TestSink>(sink);

        return pointer->getLastMessage();
    }
}