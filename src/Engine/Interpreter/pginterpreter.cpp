#include "pginterpreter.h"

#include <filesystem>
namespace fs = std::filesystem;

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

        bool stringEndWith(const std::string& fullString, const std::string& ending)
        {
            if (fullString.length() >= ending.length())
                return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
            else
                return false;
        }

    }

    ScriptImport PgInterpreter::interpretFromText(const std::string& scriptText, const CustomSysFunctions& functions)
    {
        auto ast = generateAST(scriptText);

        return _interpret(ast, functions);
        // _interpret(scriptText, tokens);
    }

    ScriptImport PgInterpreter::interpretFromFile(const std::string& scriptFile, const CustomSysFunctions& functions)
    {
        auto ast = generateASTFromFile(scriptFile);

// Todo
#ifdef DEBUG
        auto queue = ast.ast;

        while (queue.size() > 0)
        {
            auto stmt = queue.front();

            // if(stmt)
            //     std::cout << stmt->prettyPrint() << std::endl;

            queue.pop();
        }
#endif

        return _interpret(ast, functions);

        // _interpret(scriptFile, tokens);
    }

    ScriptImport PgInterpreter::interpretFromFile(const TextFile& scriptFile, const CustomSysFunctions& functions)
    {
        auto ast = generateASTFromFile(scriptFile);

        auto queue = ast.ast;

// Todo
#ifdef DEBUG
        while(queue.size() > 0)
        {
            auto stmt = queue.front();

            // if(stmt)
            //     std::cout << stmt->prettyPrint() << std::endl;

            queue.pop();
        }
#endif

        return _interpret(ast, functions);
    }

    ScriptImport PgInterpreter::getAst(const std::string& scriptName, const std::string& filePath)
    {
        // Create path to the script
        std::string fileToOpen = filePath + scriptName;

        // Append the extension if not already present
        if (not stringEndWith(scriptName, ".pg"))
            fileToOpen += ".pg";

        fs::path p {fileToOpen};

        // Check if the script file exist first
        if (not fs::exists(p))
        {
            LOG_MILE(DOM, "Couldn't load module '" << fileToOpen << "' : File doesn't exist, but it may be a system module.");
            return ScriptImport{};
        }

        // Check if an AST is available for the script
        // Checking with the relative path to insure unicity
        const auto& it = importedScripts.find(p.relative_path().string());
        
        if (it != importedScripts.end())
            return it->second;

        // Ast was not already creating, proceed to create it
        auto ast = generateASTFromFile(fileToOpen);

        return ast;
    }

    ScriptImport PgInterpreter::_interpret(const ScriptImport& script, const CustomSysFunctions& functions)
    {
        // Todo store the interpreter if a system is defined in it
        // Else just destroy it
        Interpreter *interpreter = new Interpreter(script, this);

        for (auto& it : sysFunctionTable)
        {
            it.second(interpreter, it.first);
        }

        for (const auto& it : functions.sysFunctionTable)
        {
            LOG_INFO(DOM, "Adding sys function: " << it.first);
            it.second(interpreter, it.first);
        } 

        auto env = interpreter->interpret();

        if (interpreter->hasError())
        {
            LOG_ERROR(DOM, "Interpreter error");
            return script;
        }

        if (importedScripts.find(script.name) == importedScripts.end())
            importedScripts[script.name] = script;
        
        if (interpreter->hasEcsSys())
            sysInterpreters.push_back(interpreter);
        else
            delete interpreter;

        // Todo remove this save of the env as the interpreter is created locally here and so the env become useless once we leave this function
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

        if (parser.hasError())
        {
            LOG_ERROR(DOM, "Parser error");
            return script;
        }

        if (resolver.hasError())
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
        auto name = UniversalFileAccessor::getRelativePath(file);

        auto ast = generateAST(file.data);
        ast.name = name;

        importedScripts[name] = ast;

        return ast;
    }
}