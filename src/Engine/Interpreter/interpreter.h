#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <string>

#include "expression.h"
#include "statement.h"
#include "environment.h"

namespace pg
{
    //TODO: handle different types of contexts
    class Visitor
    {
    public:
        Visitor(std::shared_ptr<Environment> environment = nullptr) : env(std::make_shared<Environment>(environment)) {}
        virtual ~Visitor() {}

        virtual std::shared_ptr<Valuable> visit(BinaryExpression *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(LogicExpression *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(UnaryExpression *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(PreFixExpression *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(PostFixExpression *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(CompoundAtom *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(Atom *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(List *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(This *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(Var *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(Assign *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(CallExpression *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(Get *expr) = 0;
        virtual std::shared_ptr<Valuable> visit(Set *expr) = 0;

        virtual void visitStatement(ExpressionStatement *stmt) = 0;
        virtual void visitStatement(VariableStatement *stmt) = 0;
        virtual void visitStatement(FunctionStatement *stmt) = 0;
        virtual void visitStatement(ClassStatement *stmt) = 0;
        virtual void visitStatement(BlockStatement *stmt) = 0;
        virtual void visitStatement(IfStatement *stmt) = 0;
        virtual void visitStatement(WhileStatement *stmt) = 0;
        virtual void visitStatement(ReturnStatement *stmt) = 0;
        virtual void visitStatement(ImportStatement *stmt) = 0;

    protected:
        std::shared_ptr<Environment> env;
    };

    class PgInterpreter;
    class Interpreter;
    class SysModule;

    class VisitorInterpreter : public Visitor
    {
    friend class Interpreter;
    friend class SysModule;
    public:
        VisitorInterpreter(PgInterpreter *interpreter, std::shared_ptr<Environment> environment, const std::unordered_map<Expression*, unsigned int>& localsList, const std::string& scriptName) : Visitor(environment), localsList(localsList), interpreter(interpreter), scriptName(scriptName) {}
        virtual ~VisitorInterpreter() {}

        virtual std::shared_ptr<Valuable> visit(BinaryExpression *expr) override;
        virtual std::shared_ptr<Valuable> visit(LogicExpression *expr) override;
        virtual std::shared_ptr<Valuable> visit(UnaryExpression *expr) override;
        virtual std::shared_ptr<Valuable> visit(PreFixExpression *expr) override;
        virtual std::shared_ptr<Valuable> visit(PostFixExpression *expr) override;
        virtual std::shared_ptr<Valuable> visit(CompoundAtom *expr) override;
        virtual std::shared_ptr<Valuable> visit(Atom *expr) override;
        virtual std::shared_ptr<Valuable> visit(List *expr) override;
        virtual std::shared_ptr<Valuable> visit(This *expr) override;
        virtual std::shared_ptr<Valuable> visit(Var *expr) override;
        virtual std::shared_ptr<Valuable> visit(Assign *expr) override;
        virtual std::shared_ptr<Valuable> visit(CallExpression *expr) override;
        virtual std::shared_ptr<Valuable> visit(Get *expr) override;
        virtual std::shared_ptr<Valuable> visit(Set *expr) override;

        virtual void visitStatement(ExpressionStatement *stmt) override;
        virtual void visitStatement(VariableStatement *stmt) override;
        virtual void visitStatement(FunctionStatement *stmt) override;
        virtual void visitStatement(ClassStatement *stmt) override;
        virtual void visitStatement(BlockStatement *stmt) override;
        virtual void visitStatement(IfStatement *stmt) override;
        virtual void visitStatement(WhileStatement *stmt) override;
        virtual void visitStatement(ReturnStatement *stmt) override;
        virtual void visitStatement(ImportStatement *stmt) override;

        inline void resetReturnFlags() { returnTriggered = false; }

    private:
        std::shared_ptr<Environment> globalContext = env;
        std::unordered_map<Expression*, unsigned int> localsList;

        friend std::shared_ptr<Valuable> executeBlock(std::queue<StatementPtr> statements, VisitorInterpreter* visitor, std::shared_ptr<Environment> environment);

        std::shared_ptr<Environment> ancestor(int distance) const;
        std::shared_ptr<Valuable> lookUpVariable(const std::string& name, const Token& token, Expression* expression) const;
        std::shared_ptr<Valuable> getAt(int distance, const std::string& name, const Token& token) const;

        void assignVariable(const Token& name, Expression* expression, std::shared_ptr<Valuable> value);
        void assignAt(int distance, const Token& name, std::shared_ptr<Valuable> value);
            
        /** Flag to indicate that a return statement was encountered */
        bool returnTriggered = false;

        /** Return value in case a return statement was encountered */
        std::shared_ptr<Valuable> returnValue;

        PgInterpreter *interpreter;

        std::string scriptName;

        // Keep a reference to all imported interpreters to keep their statement ptr valid
        std::vector<std::shared_ptr<Interpreter>> importedInterpreters;
    };

    std::shared_ptr<Valuable> executeBlock(std::queue<StatementPtr> statements, VisitorInterpreter* visitor, std::shared_ptr<Environment> environment);

    struct ScriptImport
    {
        std::queue<StatementPtr> ast;
        std::unordered_map<Expression*, unsigned int> symbols;
        std::shared_ptr<Environment> env = nullptr;
        std::string name = "";
    };

    class Interpreter
    {
    public:
        Interpreter(const ScriptImport& script, PgInterpreter *interpreter) : localsList(script.symbols), visitor(interpreter, nullptr, localsList, script.name), statements(script.ast) {};

        template<typename Functional>
        void defineSystemFunction(const std::string& name);

        std::shared_ptr<Environment> interpret();

        inline bool hasError() const { return encounteredError; }

    private:
        std::unordered_map<Expression*, unsigned int> localsList;
        VisitorInterpreter visitor;
        
        std::queue<StatementPtr> statements;
        bool encounteredError = false;
    };

    template<typename Functional>
    void Interpreter::defineSystemFunction(const std::string& name)
    {
        std::queue<ExprPtr> emptyQueue;
        Token token;
        token.text = name;

        auto function = std::make_shared<Functional>(visitor.globalContext, name, token, &visitor, emptyQueue, nullptr);
        // Todo check staticly if type Functional as a setUp function using SFINAE
        function->setUp();

        visitor.globalContext->declareValue(name, function);
    }

    struct SysListElement
    {
        SysListElement(const std::string& str, ValuablePtr type) : key(str), value(type) {}
        SysListElement(const std::string& str, std::shared_ptr<ClassInstance> type) : key(str), value(type) {}
        SysListElement(const std::string& str, const ElementType& type) : key(str), value(makeVar(type)) {}

        template<typename T>
        SysListElement(const std::string& str, T&& t) : key(str), value(makeVar(std::forward<T>(t))) {}

        std::string key;
        ValuablePtr value;
    };

    std::shared_ptr<ClassInstance> addToList(std::shared_ptr<ClassInstance> instance, const Token& token, const SysListElement& arg);

    std::shared_ptr<ClassInstance> makeList(const Function *caller, const std::initializer_list<SysListElement>& list);
}
