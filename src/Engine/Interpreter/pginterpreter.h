#pragma once

#include "interpreter.h"

#include <unordered_map>

namespace pg
{
    class PgInterpreter
    {
    public:        
        void interpretFromText(const std::string& scriptText);
        void interpretFromFile(const std::string& scriptFile);

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