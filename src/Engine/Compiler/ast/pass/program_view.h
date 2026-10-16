#pragma once

/**
 * @file program_view.h
 * @brief What a whole program defines, for AST passes that must know every definition of a name.
 *
 * The program is the script being compiled and every module it imports, found as the compiler
 * finds them (beside the script: VM::currentFileName). A module with a .pgc only cannot be read:
 * `complete` is then false and a pass relying on the view should change nothing.
 */

#include "Interpreter/statement.h"

#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>

namespace pg
{
    struct VM;

    namespace ast
    {
        struct ProgramView
        {
            std::map<std::string, int> defined;                                // Top-level `fun` definitions by name
            std::map<std::string, std::shared_ptr<FunctionStatement>> functions;
            std::set<std::string> poisoned;                                    // Assigned, declared or a class somewhere
            std::set<std::string> visited;                                     // Files read
            bool complete = true;

            // Every name that may hold script code: a `fun`, or anything written
            std::set<std::string> scriptNames() const
            {
                std::set<std::string> out = poisoned;

                for (const auto& [name, count] : defined)
                    out.insert(name);

                return out;
            }
        };

        ProgramView readProgram(VM* vm, const std::queue<StatementPtr>& statements);
    }
}
