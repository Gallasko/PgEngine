#include "pginterpreter.h"

#include "logger.h"

namespace pg
{
    namespace
    {
        const char * const DOM = "PG Interpreter";
    }

    void PgInterpreter::interpretFromText(const std::string& scriptText)
    {
        Lexer lexer;

        try
        {
            lexer.readFromText(scriptText);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(DOM, e.what());
            return;
        }

        auto tokens = lexer.getTokens();

        _interpret(scriptText, tokens);
    }

    void PgInterpreter::interpretFromFile(const std::string& scriptFile)
    {
        Lexer lexer;

        try
        {
            lexer.readFromFile(scriptFile);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(DOM, e.what());
            return;
        }

        const auto& tokens = lexer.getTokens();

        _interpret(scriptFile, tokens);
    }

    const PgInterpreter::ScriptImport& PgInterpreter::getAst(const std::string& scriptFile) const
    { 
        const auto& it = importedScripts.find(scriptFile);
        
        if(it != importedScripts.end())
            return it->second;


        // Todo remove this and replace it !
        throw std::runtime_error("No AST here");
    }

    void PgInterpreter::_interpret(const std::string& name, const std::queue<Token>& tokens)
    {
        Parser parser(tokens);

        auto ast = parser.parse();

        Resolver resolver(ast);

        auto symbolTable = resolver.resolve();

        if(parser.hasError())
        {
            LOG_ERROR(DOM, "Parser error");
            return;
        }

        if(resolver.hasError())
        {
            LOG_ERROR(DOM, "Resolver error");
            return;
        }

        // Interpreter interpreter(resolver.getStatementsList(), localsList);

//     interpreter.defineSystemFunction<ToString>("toString");
//     interpreter.defineSystemFunction<LogInfo>("logInfo");
//     interpreter.defineSystemFunction<HRClock>("mTime");

//     interpreter.interpret();

//     if(interpreter.hasError())
//     {
//         LOG_ERROR(DOM, "Interpreter error");
//         return 60;
//     }

    }

    void PgInterpreter::generateAST(const std::string& name, const std::queue<Token>& tokens)
    {

    }
}