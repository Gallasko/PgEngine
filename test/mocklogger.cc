#include "mocklogger.h"

namespace pg
{
    TestSink::TestSink()
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
        nbMessages[log.level]++;

        lastMessage = {log.message, log.level};
    }

    MockLogger::MockLogger()
    {
        sink = Logger::registerSink<TestSink>();
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