#include "pginterpreter.h"

#include "lexer.h"
#include "parser.h"
#include "resolver.h"

#include "logger.h"

// Todo to remove
#include "systemfunction.h"

namespace pg
{
    namespace
    {
        const char * const DOM = "PG Interpreter";
    }

    ScriptImport PgInterpreter::interpretFromText(const std::string& scriptText)
    {
        auto ast = generateAST(scriptText);

        return _interpret(ast);
        // _interpret(scriptText, tokens);
    }

    ScriptImport PgInterpreter::interpretFromFile(const std::string& scriptFile)
    {
        auto ast = generateASTFromFile(scriptFile);

        return _interpret(ast);

        // _interpret(scriptFile, tokens);
    }

    ScriptImport PgInterpreter::interpretFromFile(const TextFile& scriptFile)
    {
        auto ast = generateASTFromFile(scriptFile);

        return _interpret(ast);
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

    ScriptImport PgInterpreter::_interpret(const ScriptImport& script)
    {
        Interpreter interpreter(script, this);

        for(auto& it : sysFunctionTable)
        {
            it.second(&interpreter, it.first);
        }

        // Todo to remove
        // interpreter.defineSystemFunction<LogInfo>("logInfo");

        auto env = interpreter.interpret();

        if(interpreter.hasError())
        {
            LOG_ERROR(DOM, "Interpreter error");
            return script;
        }

        importedScripts[script.name] = script;
        importedScripts[script.name].env = env;

        return importedScripts[script.name];
    }

    ScriptImport PgInterpreter::generateAST(const std::string& data)
    {
        ScriptImport script;
        Lexer lexer;

        try
        {
            lexer.readFromText(data);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(DOM, e.what());
            return script;
        }

        auto tokens = lexer.getTokens();

        Parser parser(tokens);

        auto ast = parser.parse();

        Resolver resolver(ast);

        auto symbolTable = resolver.resolve();

        if(parser.hasError())
        {
            LOG_ERROR(DOM, "Parser error");
            return script;
        }

        if(resolver.hasError())
        {
            LOG_ERROR(DOM, "Resolver error");
            return script;
        }

        script.ast = ast;
        script.symbols = symbolTable;

        return script;
    }

    ScriptImport PgInterpreter::generateASTFromFile(const std::string& filename)
    {
        auto file = UniversalFileAccessor::openTextFile(filename);
        
        return generateASTFromFile(file);
    }

    ScriptImport PgInterpreter::generateASTFromFile(const TextFile& file)
    {
        auto name = UniversalFileAccessor::getFileName(file);

        auto ast = generateAST(file.data);
        ast.name = name;

        return ast;
    }
}