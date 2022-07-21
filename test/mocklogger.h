#pragma once

#include <gtest/gtest.h>

#include "logger.h"

namespace pg
{
    struct LogMessage
    {
        std::string message;
        Logger::InfoLevel level;
    };

    class TestSink : public Logger::LogSink
    {
    friend class Logger;
    public:
        TestSink();

        void resetSink();

        const std::unordered_map<Logger::InfoLevel, unsigned int>& getNbMessages() const { return nbMessages; }
        const LogMessage& getLastMessage() const { return lastMessage; }
        
        virtual ~TestSink() {}

        /** Stream operator used to get the log and print the message to the console */
        virtual void processLog(const Logger::Info& log) override;

    private:
        std::unordered_map<Logger::InfoLevel, unsigned int> nbMessages;

        LogMessage lastMessage = {"", Logger::InfoLevel::log};
    };

    class MockLogger
    {
    public:
        MockLogger();
        ~MockLogger();

        void reset();

        unsigned int getNbLog() const;
        unsigned int getNbInfo() const;
        unsigned int getNbAlert() const;
        unsigned int getNbWarning() const;
        unsigned int getNbError() const;
        unsigned int getNbCritical() const;

        const LogMessage& getLastMessage() const;

    private:
        std::shared_ptr<Logger::LogSink> sink;
    };
}