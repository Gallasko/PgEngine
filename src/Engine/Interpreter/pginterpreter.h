#pragma once

#include "lexer.h"
#include "parser.h"
#include "resolver.h"
#include "interpreter.h"

#include <map>

namespace pg
{
    class PgInterpreter
    {
    public:
        void interpret(const std::string& scriptName);
        
        void interpretFromText(const std::string& scriptText);
        void interpretFromFile(const std::string& scriptFile);

    private:
        ScriptImport getAst(const std::string& script, const std::string& filePath = "");

        ScriptImport generateAST(const std::string& data);
        ScriptImport generateASTFromFile(const std::string& filename);

        void _interpret(const std::string& name, const std::queue<Token>& tokens);

        Interpreter interpreter;

        // Todo store the absolute path of the custom made script file
        // to avoid using the AST of another file when trying to import a script
        std::unordered_map<std::string, ScriptImport> importedScripts;
    };
}