#include "logger.h"

std::vector<Logger::Info> Logger::log;

std::mutex Logger::_lock;

void Logger::printLog() const
{
    for(auto info : log)
    {
        
        
    }
}

