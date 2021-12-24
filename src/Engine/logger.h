#include <vector>

#include <memory>
#include <mutex>

#define LOG_THIS(scope, msg) Logger::_log(__LINE__, __FILE__, __FUNCTION__, 0, 0, scope, msg)
#define LOG_THIS_MEMBER(scope, msg) Logger::_log(__LINE__, __FILE__, __FUNCTION__, this, typeid(*this).name(), scope, msg)

class Logger
{
    typedef std::unique_ptr<Logger> LoggerPtr;

    struct Info
    {
        const int line;
        const char* filename;
        const char* function;
        const void* object;
        const char* objectName;

        const char* scope;
        const char* message;
    };

public:
    inline static void _log(const int line, const char* file, const char* function, const void* object, const char* objectName, const char* scope, const char* msg)
    {
        // Fonctor to use C++ scope initialisation to easely lock log pushback
        std::lock_guard<std::mutex> lock(_lock);
    
        //std::cout << line << ", " << file << ", " << function << ", " << objectName << "," << scope << ", " << msg << std::endl;

        log.push_back(Logger::Info{line, file, function, object, objectName, scope, msg});    
    }

    const LoggerPtr& getLogger() const { static LoggerPtr logger = LoggerPtr(new Logger()); return logger; } 

private:
    static std::vector<Logger::Info> log;

    static std::mutex _lock;
};