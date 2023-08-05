#include "interpreter.h"
#include "environment.h"

#include <filesystem>
namespace fs = std::filesystem;

#include "logger.h"

#include "pginterpreter.h"

namespace pg
{

    namespace
    {
        const char * DOM = "Interpreter";

        struct EnvironmentSwapper
        {
            EnvironmentSwapper(std::shared_ptr<Environment>& env, std::shared_ptr<Environment> newEnv) : env(env) { LOG_THIS_MEMBER(DOM); oldEnv = env; env = newEnv; }
            ~EnvironmentSwapper() { LOG_THIS_MEMBER(DOM); env = oldEnv; }

            std::shared_ptr<Environment> oldEnv;
            std::shared_ptr<Environment>& env;
        };
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(BinaryExpression *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto lvalue = expr->leftExpr->accept(this)->getElement();
        auto rvalue = expr->rightExpr->accept(this)->getElement();

        switch(expr->op.type)
        {
            case TokenType::MINUS:
                try
                {
                    return std::make_shared<Variable>(lvalue - rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                    
                break;

            case TokenType::PLUS:
                try
                {
                    return std::make_shared<Variable>(lvalue + rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                    
                break;

            case TokenType::STAR:
                try
                {
                    return std::make_shared<Variable>(lvalue * rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }   
                break;
            
            case TokenType::SLASH:
                try
                {
                    return std::make_shared<Variable>(lvalue / rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                break;

            case TokenType::SUP:
                try
                {
                    return std::make_shared<Variable>(lvalue > rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                    
                break;

            case TokenType::SUPEQUAL:
                try
                {
                    return std::make_shared<Variable>(lvalue >= rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                    
                break;

            case TokenType::INF:
                try
                {
                    return std::make_shared<Variable>(lvalue < rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                    
                break;

            case TokenType::INFEQUAL:
                try
                {
                    return std::make_shared<Variable>(lvalue <= rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                    
                break;

            case TokenType::EQUALEQUAL:
                try
                {
                    return std::make_shared<Variable>(lvalue == rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                    
                break;

            case TokenType::NOTEQUAL:
                try
                {
                    return std::make_shared<Variable>(lvalue != rvalue);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                    
                break;

            default:
                throw RuntimeException(expr->op, "Unknown binary operation");
                break;
        }

        throw RuntimeException(expr->op, "Unknown binary operation");
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(LogicExpression *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto lvalue = expr->leftExpr->accept(this)->getElement();

        // Shortcut if the left operand is (true for logic_or) || (false for logic_and)
        switch(expr->op.type)
        {
            case TokenType::LOGICOR:
                if (lvalue.isTrue()) return std::make_shared<Variable>(ElementType { lvalue.isTrue() });
                break;
            
            case TokenType::LOGICAND:
                if (not lvalue.isTrue()) return std::make_shared<Variable>(ElementType { lvalue.isTrue() });
                break;

            default:
                throw RuntimeException(expr->op, "Unknown logic operation");
                break;
        }

        return std::make_shared<Variable>(ElementType { expr->rightExpr->accept(this)->getElement().isTrue() });
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(UnaryExpression *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto value = expr->expr->accept(this)->getElement();

        switch(expr->op.type)
        {
            case TokenType::NOT:
                return std::make_shared<Variable>(ElementType { not value.isTrue() });
                break;

            case TokenType::MINUS:
                try
                {
                    return std::make_shared<Variable>(-value);
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                break;

            default:
                throw RuntimeException(expr->op, "Unknown unary operation");
                break;
        }

        throw RuntimeException(expr->op, "Unknown unary operation");
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(PreFixExpression *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto value = expr->expr->accept(this)->getElement();

        switch(expr->op.type)
        {
            case TokenType::INCREMENT:
                try
                {
                    // Specialization to imitate "++it" 
                    if(expr->expr->accept(this)->getType() == "IteratorInstance")
                    {
                        auto it = std::static_pointer_cast<IteratorInstance>(expr->expr->accept(this));

                        ValuableQueue emptyQueue;

                        it->get(Token{TokenType::EXPRESSION, "next", 0, 0})->getValue(emptyQueue);

                        return it->get(Token{TokenType::EXPRESSION, "current", 0, 0})->getValue(emptyQueue);
                    }

                    auto res = std::make_shared<Variable>(value + ElementType{1});

                    assignVariable(expr->name, expr, res);

                    return res;
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                break;

            case TokenType::DECREMENT:
                try
                {
                    auto res = std::make_shared<Variable>(value - ElementType{1});

                    assignVariable(expr->name, expr, res);

                    return res;
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                break;

            default:
                throw RuntimeException(expr->op, "Unknown prefix operation");
                break;
        }

        throw RuntimeException(expr->op, "Unknown prefix operation");
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(PostFixExpression *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto baseValue = expr->expr->accept(this);
        auto value = baseValue->getElement();

        switch(expr->op.type)
        {
            case TokenType::INCREMENT:
                try
                {
                    // Specialization to imitate "it++" 
                    if(baseValue->getType() == "IteratorInstance")
                    {
                        auto it = std::static_pointer_cast<IteratorInstance>(baseValue);

                        ValuableQueue emptyQueue;

                        it->get(Token{TokenType::EXPRESSION, "next", 0, 0})->getValue(emptyQueue);

                        return baseValue;
                    }

                    auto res = std::make_shared<Variable>(value + ElementType{1});

                    assignVariable(expr->name, expr, res);

                    return baseValue;
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                break;

            case TokenType::DECREMENT:
                try
                {
                    auto res = std::make_shared<Variable>(value - ElementType{1});

                    assignVariable(expr->name, expr, res);

                    return baseValue;
                }
                catch(const std::exception& e)
                {
                    throw RuntimeException(expr->op, e.what());
                }
                break;

            default:
                throw RuntimeException(expr->op, "Unknown postfix operation");
                break;
        }

        throw RuntimeException(expr->op, "Unknown postfix operation");
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(CompoundAtom *expr)
    {
        LOG_THIS_MEMBER(DOM);

        return expr->expr->accept(this);
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(Atom *expr)
    {
        LOG_THIS_MEMBER(DOM);

        return std::make_shared<Variable>(expr->value);
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(List *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto currentEnv = std::make_shared<Environment>(env);
        EnvironmentSwapper swapper(env, currentEnv);

        std::queue<ExprPtr> emptyQueue;

        auto token = expr->squareBracket;
        token.text = "List Function";

        auto instance = std::make_shared<ClassInstance>(nullptr);

        auto get = std::make_shared<AtFunction>(expr->self, currentEnv, "List Get", token, this, emptyQueue, nullptr);
        auto set = std::make_shared<SetFunction>(expr->self, currentEnv, "List Set", token, this, emptyQueue, nullptr);
        auto size = std::make_shared<SizeFunction>(expr->self, currentEnv, "List Size", token, this, emptyQueue, nullptr, instance);
        auto it = std::make_shared<IteratorFunction>(expr->self, currentEnv, "List Iterator", token, this, emptyQueue, nullptr, instance);

        std::unordered_map<std::string, std::shared_ptr<Function>> methods;

        methods["at"] = get->bind(instance);
        methods["set"] = set->bind(instance);
        methods["it"] = it;
        methods["size"] = size;

        instance->setMethods(methods);

        auto tmp = expr->entries;

        while(tmp.size() > 0)
        {
            const auto entry = tmp.front();

            auto key = entry.key->accept(this);
            auto value = entry.value->accept(this);

            instance->set(Token{TokenType::EXPRESSION, key->getElement().toString(), token.line, token.column}, value);

            tmp.pop();
        }

        return instance;
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(This *expr)
    {
        LOG_THIS_MEMBER(DOM);

        return lookUpVariable(expr->name.text, expr->name, expr);
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(Var *expr)
    {
        LOG_THIS_MEMBER(DOM);

        return lookUpVariable(expr->name.text, expr->name, expr);
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(Assign *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto value = expr->expr->accept(this);

        assignVariable(expr->name, expr, value);

        return value;
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(CallExpression *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto caller = expr->caller->accept(this);

        std::queue<ExprPtr> temp = expr->args;
        std::queue<std::shared_ptr<Valuable>> arguments;

        while(temp.size() > 0)
        {
            arguments.push(temp.front()->accept(this));

            temp.pop();
        }

        if(caller->getType() == "Function")
            return caller->getValue(arguments);
        else if(caller->getType() == "Class")
            return caller->getValue(arguments);
        else if(caller->getType() == "List")
            return caller->getValue(arguments);

        auto function = lookUpVariable(caller->getElement().toString(), expr->paren, expr);

        return function->getValue(arguments);
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(Get *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto object = expr->object->accept(this);

        if(object->getType() == "ClassInstance")
            return std::static_pointer_cast<ClassInstance>(object)->get(expr->name);
        else if(object->getType() == "IteratorInstance")
            return std::static_pointer_cast<IteratorInstance>(object)->get(expr->name);

        throw RuntimeException(expr->name, "Only instance have properties");
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(Set *expr)
    {
        LOG_THIS_MEMBER(DOM);

        auto object = expr->object->accept(this);

        if(object->getType() != "ClassInstance" and object->getType() != "IteratorInstance")
            throw RuntimeException(expr->name, "Only instance have fields");

        auto value = expr->value->accept(this); 
        std::static_pointer_cast<ClassInstance>(object)->set(expr->name, value);

        return value;
    }

    void VisitorInterpreter::visitStatement(ExpressionStatement *stmt)
    {
        LOG_THIS_MEMBER(DOM);

        auto value = stmt->expr->accept(this);
    }

    void VisitorInterpreter::visitStatement(VariableStatement *stmt)
    {
        LOG_THIS_MEMBER(DOM);

        auto name = stmt->name;
        
        std::shared_ptr<Valuable> value;
        if(stmt->expr)
            value = stmt->expr->accept(this);
        else
            value = std::make_shared<Variable>(ElementType{});

        env->declareValue(name.text, value);
    }

    void VisitorInterpreter::visitStatement(FunctionStatement *stmt)
    {
        LOG_THIS_MEMBER(DOM);

        env->declareValue(stmt->name.text, std::make_shared<Function>(env, stmt->name.text, stmt->name, this, stmt->parameters, stmt->body));
    }

    void VisitorInterpreter::visitStatement(ClassStatement *stmt)
    {
        LOG_THIS_MEMBER(DOM);

        env->declareValue(stmt->name.text, nullptr);
        
        std::unordered_map<std::string, std::shared_ptr<Function>> methods;

        auto temp = stmt->methods;

        while(temp.size() > 0)
        {
            auto funStmt = temp.front();
            methods[funStmt->name.text] = std::make_shared<Function>(env, funStmt->name.text, funStmt->name, this, funStmt->parameters, funStmt->body);
        
            temp.pop();
        }

        auto klass = std::make_shared<Class>(env, stmt->name.text, stmt->name, this, methods);
        env->assignValue(stmt->name.text, stmt->name, klass);
    }

    void VisitorInterpreter::visitStatement(BlockStatement *stmt)
    {
        LOG_THIS_MEMBER(DOM);

        executeBlock(stmt->statements, this, std::make_shared<Environment>(env));
    }

    void VisitorInterpreter::visitStatement(IfStatement *stmt)
    {
        LOG_THIS_MEMBER(DOM);

        if(not returnTriggered and stmt->condition->accept(this)->getElement().isTrue())
        {
            stmt->thenBranch->accept(this);
        }
        else if (stmt->elseBranch)
        {
            stmt->elseBranch->accept(this);
        }
    }

    void VisitorInterpreter::visitStatement(WhileStatement *stmt)
    {
        LOG_THIS_MEMBER(DOM);

        while(not returnTriggered and stmt->condition->accept(this)->getElement().isTrue())
        {
            stmt->body->accept(this);
        }
    }

    void VisitorInterpreter::visitStatement(ReturnStatement *stmt)
    {
        LOG_THIS_MEMBER(DOM);

        if(stmt->value)
            returnValue = stmt->value->accept(this);

        returnTriggered = true;
    }

    void VisitorInterpreter::visitStatement(ImportStatement *stmt)
    {
        LOG_THIS_MEMBER(DOM);

        auto tmpImports = stmt->imports;

        if(stmt->isNamed)
        {
            LOG_ERROR(DOM, "Named imports are not supported yet.");
            // auto import = tmpImports.front();
            // auto importName = import->getName();
        }
        else
        {
            while(tmpImports.size() > 0)
            {
                auto import = tmpImports.front();

                // Resolve import name
                auto importName = import->accept(this)->getElement().toString();

                std::string scriptPath = "";

                fs::path p {scriptName};

                // Check if the current script is a file
                if (fs::exists(p))
                {
                    // Get the path of the current script file
                    scriptPath = p.relative_path().remove_filename().string();
                }

                // Get the Ast of the script that we try to import
                auto scriptAst = interpreter->getAst(importName, scriptPath);

                if(scriptAst.name == "")
                {
                    if(interpreter->isSysModule(importName))
                    {
                        for(auto& it : interpreter->sysModuleTable[importName])
                        {
                            auto function = it.second(this, it.first);

                            globalContext->declareValue(it.first, function);
                        }
                    }
                }
                else
                {
                    // Create an interpreter to interpret it
                    auto importedInterpreter = std::make_shared<Interpreter>(scriptAst, interpreter);

                    // Add all system function to the imported interpreter
                    for(auto& it : interpreter->sysFunctionTable)
                    {
                        it.second(importedInterpreter.get(), it.first);
                    }

                    // Interpret the imported script to resolve all symbols
                    auto script = importedInterpreter->interpret();

                    // Check for any errors
                    if(importedInterpreter->hasError())
                        throw RuntimeException(stmt->name, "Imported module as some errors");

                    // Add all globals symbols from the imported script to this one
                    for(const auto& element : script->variableTable)
                    {
                        globalContext->declareValue(element.first, element.second);
                    }

                    // Store a ref to the imported script interpreter to not lose the ref to the symbols
                    importedInterpreters.push_back(importedInterpreter);
                }

                tmpImports.pop();
            }
        }
    }

    std::shared_ptr<Environment> VisitorInterpreter::ancestor(int distance) const
    {
        LOG_THIS_MEMBER(DOM);

        auto currentEnv = env;
        
        for(int i = 0; i < distance; i++)
            currentEnv = currentEnv->getEnv();

        return currentEnv;
    }

    std::shared_ptr<Valuable> VisitorInterpreter::lookUpVariable(const std::string& name, const Token& token, Expression* expression) const
    {
        LOG_THIS_MEMBER(DOM);

        if(localsList.empty())
            return globalContext->getValue(name, token);

        auto it = localsList.find(expression);

        if(it != localsList.end())
            return getAt(it->second, name, token);
        else
        {
            return globalContext->getValue(name, token);
        } 
    }

    std::shared_ptr<Valuable> VisitorInterpreter::getAt(int distance, const std::string& name, const Token& token) const
    {
        LOG_THIS_MEMBER(DOM);

        return ancestor(distance)->getValue(name, token);
    }

    void VisitorInterpreter::assignVariable(const Token& name, Expression* expression, std::shared_ptr<Valuable> value)
    {
        LOG_THIS_MEMBER(DOM);

        if(localsList.empty())
            return globalContext->assignValue(name.text, name, value);
        
        auto it = localsList.find(expression);

        if(it != localsList.end())
            return assignAt(it->second, name, value);
        else
            return globalContext->assignValue(name.text, name, value);
    }

    void VisitorInterpreter::assignAt(int distance, const Token& name, std::shared_ptr<Valuable> value)
    {
        LOG_THIS_MEMBER(DOM);

        ancestor(distance)->assignValue(name.text, name, value);
    }

    std::shared_ptr<Valuable> executeBlock(std::queue<StatementPtr> statements, VisitorInterpreter* visitor, std::shared_ptr<Environment> environment)
    {
        LOG_THIS(DOM);

        EnvironmentSwapper swapper(visitor->env, environment);

        while(statements.size() > 0)
        {
            auto statement = statements.front();

            if(statement)
            {
                statement->accept(visitor);
            }

            if(visitor->returnTriggered)
            {
                visitor->returnTriggered = true;
                return visitor->returnValue;
            }

            statements.pop();
        }

        return std::make_shared<Variable>(ElementType{0});
    }

    std::shared_ptr<Environment> Interpreter::interpret()
    {
        LOG_THIS_MEMBER(DOM);

        while(not statements.empty())
        {
            auto stmt = statements.front();
            statements.pop();

            try
            {
                if(stmt)
                    stmt->accept(&visitor);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(DOM, e.what());
                encounteredError = true;
            }
        }

        return visitor.env;
    }

    std::shared_ptr<ClassInstance> addToList(std::shared_ptr<ClassInstance> instance, const Token& token, const SysListElement& arg)
    {
        instance->set(Token{TokenType::EXPRESSION, arg.key, token.line, token.column}, std::make_shared<Variable>(arg.value));

        return instance;
    }

    std::shared_ptr<ClassInstance> makeList(const Function *caller, const std::initializer_list<SysListElement>& list)
    {
        std::queue<ExprPtr> emptyQueue;

        auto token = caller->getToken();
        token.text = "List Function";

        auto instance = std::make_shared<ClassInstance>(nullptr);

        auto self = std::make_shared<This>(Token{TokenType::EXPRESSION, "this", token.line, token.column});

        auto get = std::make_shared<AtFunction>(self, caller->getEnv(), "List Get", token, caller->getVisitor(), emptyQueue, nullptr);
        auto set = std::make_shared<SetFunction>(self, caller->getEnv(), "List Set", token, caller->getVisitor(), emptyQueue, nullptr);
        auto size = std::make_shared<SizeFunction>(self, caller->getEnv(), "List Size", token, caller->getVisitor(), emptyQueue, nullptr, instance);
        auto it = std::make_shared<IteratorFunction>(self, caller->getEnv(), "List Iterator", token, caller->getVisitor(), emptyQueue, nullptr, instance);

        std::unordered_map<std::string, std::shared_ptr<Function>> methods;

        methods["at"] = get->bind(instance);
        methods["set"] = set->bind(instance);
        methods["it"] = it;
        methods["size"] = size;

        instance->setMethods(methods);

        for(const auto& item : list)
        {
            addToList(instance, token, item);
        }

        return instance;
    }

}
