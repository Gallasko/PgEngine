#pragma once

#include <vector>

#include <memory>
#include <mutex>

#ifdef DEBUG
#define LOG_THIS(scope, msg) Logger::_log(__LINE__, __FILE__, __FUNCTION__, 0, 0, scope, msg, Logger::InfoLevel::log)
#define LOG_THIS_MEMBER(scope, msg) Logger::_log(__LINE__, __FILE__, __FUNCTION__, this, typeid(*this).name(), scope, msg, Logger::InfoLevel::log)
#else
#define LOG_THIS(scope, msg) 
#define LOG_THIS_MEMBER(scope, msg) 
#endif

// For testing purposes / make it a console listener
#include <iostream>

class Logger
{
public:
    enum class InfoLevel
    {
        log = 0,
        info = 1,
        alert = 2,
        warning = 3,
        error = 4,
        critical = 5
    };

private:
    struct Info
    {
        const int line;
        const char* filename;
        const char* function;
        const void* object;
        const char* objectName;

        const char* scope;
        const char* message;

        const InfoLevel level;
    };

    typedef std::unique_ptr<Logger> LoggerPtr;


public:
    /**
     * @brief Main function used to register a log message
     * 
     * @param line          Line where the log message happened
     * @param file          File where the log message happened
     * @param function      Function where the log message happened
     * @param object        A reference to the object where the log message happened
     * @param objectName    Name of the object where the log message happened
     * @param scope         Scope of the message for filtering and priority
     * @param msg           Message to be logged
     * @param level         Level of emergency of the message
     */
    inline static void _log(const int line, const char* file, const char* function, const void* object, const char* objectName, const char* scope, const char* msg, const Logger::InfoLevel& level)
    {
        // Fonctor to use C++ scope initialisation to easely lock log pushback
        std::lock_guard<std::mutex> lock(_lock);

        std::cout << line << ", " << file << ", " << function << ", " << objectName << "," << scope << ", " << msg << ", " << static_cast<int>(level) << std::endl;

        log.push_back(Logger::Info{line, file, function, object, objectName, scope, msg, level});    
    }

    /**
     * @brief Get the reference of the unique Logger instance
     * 
     * @return a pointer to the logger instance
     * 
     * This function create an Logger object the first time it is called and then return an unique reference to this object 
     */
    inline static const LoggerPtr& getLogger() { static LoggerPtr logger = LoggerPtr(new Logger()); return logger; } 

    void printLog() const;

private:
    static std::vector<Logger::Info> log;

    static std::mutex _lock;
};