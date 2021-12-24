#include "logger.h"

std::vector<Logger::Info> Logger::log;

std::mutex Logger::_lock;

