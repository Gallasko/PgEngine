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
        struct ScriptImport
        {
            std::queue<StatementPtr> ast;
            std::unordered_map<Expression*, unsigned int> symbols;
            std::shared_ptr<Environment> env = nullptr;
        };

    public:
        void interpretFromText(const std::string& scriptText);
        void interpretFromFile(const std::string& scriptFile);

        const ScriptImport& getAst(const std::string& script) const;

    private:
        void _interpret(const std::string& name, const std::queue<Token>& tokens);

        void generateAST(const std::string& name, const std::queue<Token>& tokens);

        Interpreter interpreter;

        mutable std::map<std::string, ScriptImport> importedScripts;
    };
}