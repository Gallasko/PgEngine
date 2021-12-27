#include "logger.h"

std::vector<Logger::LogSinkPtr> Logger::sinks;

std::mutex Logger::_lock;

// Only used for the console sink
#include <iostream>

void TerminalSink::operator<<(const Logger::Info& log)
{
    if(not ignoreNonErrors and log.level == Logger::InfoLevel::log)
        std::cout << log.line << ", " << log.filename << ", " << log.function << ", " << log.objectName << "," << log.scope << ", " << log.message << ", " << static_cast<int>(log.level) << std::endl;
}