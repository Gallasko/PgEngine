#pragma once

#include <string>
#include <queue>

#include "expression.h"

namespace pg
{

    class Statement
    {
    public:
        Statement() {}
        virtual ~Statement() {}

        virtual void accept(Visitor* visitor) = 0;
        virtual std::string prettyPrint() const = 0;
        virtual std::string getType() const = 0;
    };

    typedef std::shared_ptr<Statement> StatementPtr;

    struct ExpressionStatement : public Statement
    {
        ExpressionStatement(ExprPtr expression) : Statement(), expr(expression) {}
        ~ExpressionStatement() {}

        virtual void accept(Visitor* visitor);
        virtual std::string prettyPrint() const { return "Expression statement: " + expr->prettyPrint(); }
        virtual std::string getType() const { return "ExpressionStatement"; }

        ExprPtr expr;
    };

    struct VariableStatement : public Statement
    {
        VariableStatement(const Token& name, ExprPtr expression) : Statement(), name(name), expr(expression) {}
        ~VariableStatement() {}

        virtual void accept(Visitor* visitor);
        virtual std::string prettyPrint() const { auto initializerValue = expr != nullptr ? expr->prettyPrint() : "null"; return "Assignment statement, set variable :" + name.text + ", to value: " + initializerValue; }
        virtual std::string getType() const { return "VariableStatement"; }

        Token name;
        ExprPtr expr;
    };

    struct FunctionStatement : public Statement
    {
        FunctionStatement(const Token& name, const std::queue<ExprPtr>& parameters, StatementPtr body) : Statement(), name(name), parameters(parameters), body(body) {}
        ~FunctionStatement() {}

        virtual void accept(Visitor* visitor);
        virtual std::string prettyPrint() const {auto p = parameters; std::string res = ""; while(p.size() > 0) { res += p.front()->prettyPrint() + ", "; p.pop();} return "Function statement :" + name.text + " with parameters: " + res; }
        virtual std::string getType() const { return "FunctionStatement"; }

        Token name;
        std::queue<ExprPtr> parameters;
        StatementPtr body;
    };

    struct ClassStatement : public Statement
    {
        ClassStatement(const Token& name, const std::queue<std::shared_ptr<FunctionStatement>>& methods) : Statement(), name(name), methods(methods) {}
        ~ClassStatement() {}

        virtual void accept(Visitor* visitor);
        virtual std::string prettyPrint() const { return "Class statement : " + name.text; }
        virtual std::string getType() const { return "ClassStatement"; }

        Token name;
        std::queue<std::shared_ptr<FunctionStatement>> methods;
    };

    struct BlockStatement : public Statement
    {
        BlockStatement(const std::queue<StatementPtr>& statements) : Statement(), statements(statements) {}
        ~BlockStatement() {}

        virtual void accept(Visitor* visitor);
        virtual std::string prettyPrint() const { return "Statement block"; }
        virtual std::string getType() const { return "BlockStatement"; }

        std::queue<StatementPtr> statements;
    };

    struct IfStatement : public Statement
    {
        IfStatement(ExprPtr condition, StatementPtr thenBranch, StatementPtr elseBranch) : Statement(), condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
        ~IfStatement() {}

        virtual void accept(Visitor* visitor);
        virtual std::string prettyPrint() const { return "If statement, with condition: " + condition->prettyPrint(); } 
        virtual std::string getType() const { return "IfStatement"; }

        ExprPtr condition;
        StatementPtr thenBranch;
        StatementPtr elseBranch;
    };

    struct WhileStatement : public Statement
    {
        WhileStatement(ExprPtr condition, StatementPtr body) : Statement(), condition(condition), body(body) {}
        ~WhileStatement() {}

        virtual void accept(Visitor* visitor);
        virtual std::string prettyPrint() const { return "while statement, with condition: " + condition->prettyPrint(); }
        virtual std::string getType() const { return "WhileStatement"; }

        ExprPtr condition;
        StatementPtr body;
    };

    struct ReturnStatement : public Statement
    {
        ReturnStatement(const Token& name, ExprPtr value) : Statement(), name(name), value(value) {}
        ~ReturnStatement() {}

        virtual void accept(Visitor* visitor);
        virtual std::string prettyPrint() const { return "Return statement with value: " + value->prettyPrint(); }
        virtual std::string getType() const { return "ReturnStatement"; }

        Token name;
        ExprPtr value;
    };

    struct ImportStatement : public Statement
    {
        ImportStatement(const Token& name, const std::queue<ExprPtr>& imports, ExprPtr importName) : Statement(), name(name), imports(imports), importName(importName) {}
        ~ImportStatement() {}

        virtual void accept(Visitor* visitor);
        virtual std::string prettyPrint() const {auto p = imports; std::string res = ""; while(p.size() > 0) { res += "[" + p.front()->prettyPrint() + "], "; p.pop();} return "Importing: " + res + (importName != nullptr ? "as" + importName->prettyPrint() : ""); }
        virtual std::string getType() const { return "ImportStatement"; }

        Token name;
        std::queue<ExprPtr> imports;
        ExprPtr importName;
    };

}