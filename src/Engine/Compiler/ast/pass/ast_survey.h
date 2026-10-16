#pragma once

/**
 * @file ast_survey.h
 * @brief AST "pass" that changes nothing and reports what other passes could do.
 *
 * Enabled with the environment variable PG_AST_SURVEY=<file> (EntitySystem::setupVm adds it then):
 * every script compiled by the AST front-end appends to that file
 *
 *   - its functions whose body is a single `return <expression>`, with how often the file calls
 *     each of them (inlining candidates: `fun has(t, k) { return contain(t, k) }`);
 *   - the functions it calls most, by name;
 *   - its loops whose condition calls `len(x)` while the body never changes x (no assignment to
 *     x, no push / insert / removeAt / pop on it): the length could be read once;
 *   - field reads repeated inside one loop body on a variable the body does not assign
 *     (`a.requires` read four times): candidates for a hidden local;
 *   - how many of its loops contain a call (the loop-invariant hoisting pass leaves those alone).
 *
 * Read-only: it never transforms the statements, and returns false.
 */

#include "../ast_pass.h"

namespace pg
{
    class AstSurveyPass : public AstPass
    {
    public:
        explicit AstSurveyPass(const std::string& outPath) : outPath(outPath) {}

        std::string getName() const override { return "AstSurvey"; }

        bool runPass(VM* vm, std::queue<StatementPtr>& statements) override;

    private:
        std::string outPath;
    };
}
