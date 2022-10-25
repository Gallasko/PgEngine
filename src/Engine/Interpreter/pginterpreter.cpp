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

        // _interpret(scriptText, tokens);
    }

    void PgInterpreter::interpretFromFile(const std::string& scriptFile)
    {
        auto ast = generateASTFromFile(scriptFile);

        // _interpret(scriptFile, tokens);
    }

    ScriptImport PgInterpreter::getAst(const std::string& scriptName, const std::string& filePath)
    { 
        // Check if an AST is available for the script
        const auto& it = importedScripts.find(scriptName);
        
        if(it != importedScripts.end())
            return it->second;

        // Look for the a file in the current directory of the script
        // and try to generate his AST
        auto ast = generateASTFromFile(filePath + "/" + scriptName + ".pg");

        // Todo: check how to store this AST (maybe be absolute path instead of relative path)
        // to avoid recreating the same AST for the same script
        return ast;

        /*
            // Check again if an AST is available for the script because it just got generated
            const auto& it = importedScripts.find(scriptName);
        
            if(it != importedScripts.end())
                return it->second;

            LOG_ERROR(DOM, Strfy() << "No AST found for script " << scriptName);

            // Send an empty Import indicating that no AST is available
            return PgInterpreter::ScriptImport{};
    
        */

    }

    void PgInterpreter::_interpret(const std::string& name, const std::queue<Token>& tokens)
    {
        

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

    ScriptImport PgInterpreter::generateAST(const std::string& data)
    {
        Lexer lexer;

        try
        {
            lexer.readFromText(data);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(DOM, e.what());
            return;
        }

        auto tokens = lexer.getTokens();

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

        return {ast, symbolTable};
    }

    ScriptImport PgInterpreter::generateASTFromFile(const std::string& filename)
    {
        auto file = UniversalFileAccessor::openTextFile(filename);
        auto name = UniversalFileAccessor::getFileName(file);

        return generateAST(file.data);
    }
}