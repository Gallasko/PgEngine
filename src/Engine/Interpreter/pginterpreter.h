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
        void interpret(const std::string& scriptFile);

    private:
        std::map<std::string, std::queue<StatementPtr>> listOfStatement;
    };
}