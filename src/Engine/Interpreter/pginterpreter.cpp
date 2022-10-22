#include "pginterpreter.h"

#include "logger.h"

namespace pg
{
    namespace
    {
        const char * const DOM = "PG Interpreter";
    }

    void PgInterpreter::interpret(const std::string& scriptFile)
    {
        Lexer lexer;

        try
        {
            lexer.readFile(scriptFile);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(DOM, e.what());
            return;
        }

        auto token = lexer.getTokens();

        Parser parser(token);
    }
}