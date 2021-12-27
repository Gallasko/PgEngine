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

/**
 * @class Logger
 * 
 * Main logging class responsible of managing logging sinks and print various info of the code
 * 
 */
class Logger
{
public:
    /**
     * @enum LogLevel
     * 
     * A enum class holding the emergency level of the log
     * 
     */
    enum class InfoLevel
    {
        log = 0,                    ///< Log level used anywhere for basic logging
        info = 1,                   ///< Info level used to print some important and informative message about the execution of the code
        alert = 2,                  ///< Alert level used to alert the dev of weird branchings that can affect the output 
        warning = 3,                ///< Warning level used to warn the developer of an error that is non blocking 
        error = 4,                  ///< Error level used to tell the developer of an error that is blocking and may need a restart of a component
        critical = 5                ///< Critical level used to tell the developer of an error that is critical to the integrity of the application and need a full reboot of it
    };

    /**
     * @struct Info
     * 
     * A structure holding all the information about a log message
     * 
     */
    struct Info
    {
        const int line;             ///< The line number of the message
        const char* filename;       ///< The name of the file where the log message happened
        const char* function;       ///< The name of the function where the log message happened
        const void* object;         ///< A pointer to the object where the log message happened
        const char* objectName;     ///< The name of the object class where the log message happened

        const char* scope;          ///< The scope of the log
        const char* message;        ///< The message string

        const InfoLevel level;      ///< The emergency level of the log
    };

    /**
     * @class LogSink
     * 
     * Pure virtual class to create derive classes used to be end points of the logs system
     * 
     */
    class LogSink
    {
    friend class Logger;
    public:
        /** Virtual destructor for LogSink's children */
        virtual ~LogSink() {}

        /** Stream operator used to push the log into the sink */
        virtual void operator<<(const Logger::Info& log) = 0;
    };

    // Typedefs

    /** Logger unique pointer type definition */
    typedef std::unique_ptr<Logger> LoggerPtr;
    /** LogSink unique pointer type definition */
    typedef std::unique_ptr<LogSink> LogSinkPtr;

public:
    /**
     * @brief Register a sink to dumb log into
     * 
     * @tparam Sink Which type of sink to be initialised
     * @tparam Args Variadic list of arguments to initialize the sink
     * @param args Arguments of the sink to be initialised
     * @return an integer which is the pos of the sink in the sink vector
     */
    template <typename Sink, typename... Args>
    inline static int registerSink(Args... args);

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

        // Call all the sink registered and push the received message to them
        for(const auto& sink : sinks)
            *sink << Logger::Info{line, file, function, object, objectName, scope, msg, level};    
    }

    /**
     * @brief Get the reference of the unique Logger instance
     * 
     * @return a pointer to the logger instance.
     * 
     * This function create an Logger object the first time it is called and then return an unique reference to this object
     *
     */
    inline static const LoggerPtr& getLogger() { static LoggerPtr logger = LoggerPtr(new Logger()); return logger; } 

// TODO commit log to file when a certain threashold of message is passed
// Define in #define max log length
private:
    /** Current batch of log */
    static std::vector<LogSinkPtr> sinks;
    
    /** Mutex for pushing and accessing logs */ 
    static std::mutex _lock;
};

template <typename Sink, typename... Args>
int Logger::registerSink(Args... args)
{
    // Fonctor to use C++ scope initialisation to easely lock log pushback
    std::lock_guard<std::mutex> lock(_lock);

    // Push back the sink in the vector
    sinks.push_back(LogSinkPtr(new Sink(args...)));

    return sinks.size() - 1;
}

// TODO create a filter function for the sinks to escape message depending on the scope
class TerminalSink : public Logger::LogSink
{
friend class Logger;
public:
    TerminalSink() : ignoreNonErrors(false) {}
    TerminalSink(bool ignoreNonErrors) : ignoreNonErrors(ignoreNonErrors) {}
    
    virtual ~TerminalSink() {}

    /** Stream operator used to get the log and print the message to the console */
    virtual void operator<<(const Logger::Info& log);

private:
    bool ignoreNonErrors;
};