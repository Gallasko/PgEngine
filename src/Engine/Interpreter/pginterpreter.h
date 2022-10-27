#pragma once

#include "interpreter.h"

#include <map>
#include <unordered_map>

namespace pg
{
    class PgInterpreter
    {
    public:        
        void interpretFromText(const std::string& scriptText);
        void interpretFromFile(const std::string& scriptFile);

    protected:
        typedef void(*sysFunction)(Interpreter*, const std::string&);

        template <typename Functional>
        void addSystemFunction(const std::string& name)
        {
            sysFunctionTable.emplace(name, [](Interpreter *interpreter, const std::string& sysName){ interpreter->defineSystemFunction<Functional>(sysName); });
        }

        std::map<std::string, sysFunction> sysFunctionTable;

    private:
        ScriptImport getAst(const std::string& script, const std::string& filePath = "");

        ScriptImport generateAST(const std::string& data);
        ScriptImport generateASTFromFile(const std::string& filename);

        void _interpret(const ScriptImport& script);

        // Todo store the absolute path of the custom made script file
        // to avoid using the AST of another file when trying to import a script
        std::unordered_map<std::string, ScriptImport> importedScripts;
    };
}