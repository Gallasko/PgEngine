#pragma once

#include <unordered_map>
#include <vector>

#include <memory>
#include <mutex>

#include <string>

//TODO Create an object to utilize the ctor/dtor and know when we enter and exit the function called
//     instead of only calling the _log() at the start of the function

#define _CONCAT_(x, y) x ## y
#define CONCAT(x, y) _CONCAT_(x, y)

#define _LOG(value) pg::Logger::Logging CONCAT(_anonymous, __LINE__) = value

#ifdef DEBUG
#define LOG_THIS(scope) _LOG(pg::Logger::_log(__LINE__, __FILE__, __func__, 0, 0, scope, "", pg::Logger::InfoLevel::log))
#define LOG_THIS_MEMBER(scope) _LOG(pg::Logger::_log(__LINE__, __FILE__, __func__, this, typeid(*this).name(), scope, "", pg::Logger::InfoLevel::log))
#define LOG_INFO(scope, msg) pg::Logger::_single_log(__LINE__, __FILE__, __func__, 0, 0, scope, msg, pg::Logger::InfoLevel::info)
#define LOG_ERROR(scope, msg) pg::Logger::_single_log(__LINE__, __FILE__, __func__, 0, 0, scope, msg, pg::Logger::InfoLevel::error)
#else
#define LOG_THIS(scope) 
#define LOG_THIS_MEMBER(scope)
#define LOG_INFO(scope, msg)
#define LOG_ERROR(scope, msg)
#endif

//TODO Filter only filter log, info, alert and warning level message -> error and critical are not filtered !
//TODO if a log is whitelisted it is shown even if it/was blacklisted

namespace pg
{
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
            log      = 0,               ///< Log level used anywhere for basic logging
            info     = 1,               ///< Info level used to print some important and informative message about the execution of the code
            alert    = 2,               ///< Alert level used to alert the dev of weird branchings that can affect the output 
            warning  = 3,               ///< Warning level used to warn the developer of an error that is non blocking 
            error    = 4,               ///< Error level used to tell the developer of an error that is blocking and may need a restart of a component
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

        class Logging
        {
        friend class Logger;
        public:
            Logging(const int line, const char* file, const char* function, const void* object, const char* objectName, const char* scope, const char* msg, const Logger::InfoLevel& level)
             : line(line), file(file), function(function), object(object), objectName(objectName), scope(scope), msg(msg), level(level)
            {
                auto& logger = Logger::getLogger();
                
                // Fonctor to use C++ scope initialisation to easely lock log pushback
                std::lock_guard<std::mutex> lock(logger->_lock);

                const auto log = Logger::Info{line, file, ("Enter in: '" + std::string(function) + "'").c_str(), object, objectName, scope, msg, level};

                // Call all the sink registered and push the received message to them
                for(const auto& sink : logger->sinks)
                    *sink << log; 
            }

            ~Logging()
            {
                auto& logger = Logger::getLogger();

                // Fonctor to use C++ scope initialisation to easely lock log pushback
                std::lock_guard<std::mutex> lock(logger->_lock);

                const auto log = Logger::Info{line, file, ("Exit out: '" + std::string(function) + "'").c_str(), object, objectName, scope, msg, level};

                // Call all the sink registered and push the received message to them
                for(const auto& sink : logger->sinks)
                    *sink << log;  
            }

            const int line;
            const char* file;
            const char* function;
            const void* object;
            const char* objectName;
            const char* scope;
            const char* msg;
            const Logger::InfoLevel& level;
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
            class Filter
            {
            public:
                virtual ~Filter() {}

                /**
                 * @brief Check if the log need to be filtered
                 * 
                 * @param log The log to be checked
                 * @return true if the log need to be filtered out and false if the filter doesn t apply to the log
                 */
                virtual bool isFiltered(const Logger::Info& log) const = 0;
            private:
            };

        public:
            class FilterFile : public Filter
            {
            public:
                FilterFile(const std::string& filename, bool blacklisted = true) : filename(filename), blacklisted(blacklisted) {}
                
                inline virtual bool isFiltered(const Logger::Info& log) const
                {
                    return blacklisted ? log.filename == filename : log.filename != filename; 
                }

            private:
                std::string filename;
                bool blacklisted;
            };

            class FilterFunction : public Filter
            {
            public:
                FilterFunction(const std::string& function, bool blacklisted = true) : function(function), blacklisted(blacklisted) {}
                
                inline virtual bool isFiltered(const Logger::Info& log) const
                {
                    return blacklisted ? log.function == function : log.function != function; 
                }

            private:
                std::string function;
                bool blacklisted;
            };

            // TODO check if this function is relevant and if it doesn t crash if ptr a non existant
            class FilterObject : public Filter
            {
            public:
                FilterObject(void* object, bool blacklisted = true) : object(object), blacklisted(blacklisted) {}
                
                inline virtual bool isFiltered(const Logger::Info& log) const
                {
                    if (object == nullptr or log.object == nullptr)
                        return true;
    
                    return blacklisted ? log.object == object : log.object != object; 
                }

            private:
                void* object;
                bool blacklisted;
            };

            class FilterObjectName : public Filter
            {
            public:
                FilterObjectName(const std::string& objectName, bool blacklisted = true) : objectName(objectName), blacklisted(blacklisted) {}
                
                inline virtual bool isFiltered(const Logger::Info& log) const
                {
                    return blacklisted ? log.objectName == objectName : log.objectName != objectName; 
                }

            private:
                std::string objectName;
                bool blacklisted;
            };

            class FilterScope : public Filter
            {
            public:
                FilterScope(const std::string& scope, bool blacklisted = true) : scope(scope), blacklisted(blacklisted) {}
                
                inline virtual bool isFiltered(const Logger::Info& log) const
                {
                    return blacklisted ? log.scope == scope : log.scope != scope; 
                }

            private:
                std::string scope;
                bool blacklisted;
            };

            class FilterLogLevel : public Filter
            {
            public:
                FilterLogLevel(const Logger::InfoLevel& level, bool blacklisted = true) : level(level), blacklisted(blacklisted) {}
                
                inline virtual bool isFiltered(const Logger::Info& log) const
                {
                    return blacklisted ? log.level == level : log.level != level; 
                }

            private:
                Logger::InfoLevel level;
                bool blacklisted;
            };

            // Todo delete all filters registered
            /** Virtual destructor for LogSink's children */
            virtual ~LogSink() {}

            // Todo: check if a filter is not already set with this filtername
            // Todo: do a function that remove the filter
            // Todo: when adding a filter with whitelist activated (blacklisted set to false) => make all call to operator<< only accept log if it is in the whitelist
            inline void addFilter(const std::string& filterName, Filter* filter) { filters[filterName] = filter; }

            /** Stream operator used to push the log into the sink */
            virtual void operator<<(const Logger::Info& log);

        private:
            virtual void processLog(const Logger::Info& log) = 0;

            // Todo delete all the element in it
            std::unordered_map<std::string, Filter*> filters;
        };

        // Typedefs

        /** Logger unique pointer type definition */
        typedef std::unique_ptr<Logger> LoggerPtr;
        /** LogSink unique pointer type definition */
        typedef std::shared_ptr<LogSink> LogSinkPtr;

    public:
        /**
         * @brief Register a sink to dumb log into
         * 
         * @tparam Sink Which type of sink to be initialised
         * @tparam Args Variadic list of arguments to initialize the sink
         * @param args Arguments of the sink to be initialised
         * @return a pointer to the sink instance registered 
         */
        template <typename Sink, typename... Args>
        inline static std::shared_ptr<Logger::LogSink> registerSink(Args... args);

        static void removeSink(std::shared_ptr<Logger::LogSink> sink);

        /**
         * @brief Main function used to register a single log message
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
        inline static void _single_log(const int line, const char* file, const char* function, const void* object, const char* objectName, const char* scope, const char* msg, const Logger::InfoLevel& level)
        {
            // Fonctor to use C++ scope initialisation to easely lock log pushback
            std::lock_guard<std::mutex> lock(_lock);

            const auto log = Logger::Info{line, file, ("In function: '" + std::string(function) + "'").c_str(), object, objectName, scope, msg, level};

            // Call all the sink registered and push the received message to them
            for(const auto& sink : sinks)
                *sink << log;
        }

         /**
         * @brief Overload function used to register a single log message
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
        inline static void _single_log(const int line, const char* file, const char* function, const void* object, const char* objectName, const char* scope, const std::string& msg, const Logger::InfoLevel& level)
        {
            _single_log(line, file, function, object, objectName, scope, msg.c_str(), level);
        }

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
        inline static Logging _log(const int line, const char* file, const char* function, const void* object, const char* objectName, const char* scope, const char* msg, const Logger::InfoLevel& level)
        {
            return Logging(line, file, function, object, objectName, scope, msg, level);
        }

        /**
         * @brief Overload function used to register a log message
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
        inline static Logging _log(const int line, const char* file, const char* function, const void* object, const char* objectName, const char* scope, const std::string& msg, const Logger::InfoLevel& level)
        {
            // Call the log function
            return _log(line, file, function, object, objectName, scope, msg.c_str(), level);
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

        /**
         * @brief Get the number of sinks currently attached to the logger subsystem
         * 
         * @return the number of sink
         */
        inline static unsigned int getNbSink() { return sinks.size(); }

    // TODO commit log to file when a certain threashold of message is passed
    // Define in #define max log length
    private:
        /** Current batch of log */
        static std::vector<LogSinkPtr> sinks;
        
        /** Mutex for pushing and accessing logs */ 
        static std::mutex _lock;
    };

    template <typename Sink, typename... Args>
    std::shared_ptr<Logger::LogSink> Logger::registerSink(Args... args)
    {
        // Fonctor to use C++ scope initialisation to easely lock log pushback
        std::lock_guard<std::mutex> lock(_lock);

        // Create an unique reference to the sink created
        std::shared_ptr<Logger::LogSink> sink = std::make_shared<Sink>(args...);

        // Push back the sink in the vector
        sinks.push_back(sink);

        return sink;
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
        virtual void processLog(const Logger::Info& log) override;

    private:
        /** Flag indicating whether we should ignore errors or not */
        bool ignoreNonErrors;
    };
}