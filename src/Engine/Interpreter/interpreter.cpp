#include "interpreter.h"
#include "environment.h"

#include "logger.h"

namespace pg
{

    namespace
    {
        const char * DOM = "Interpreter";

        struct EnvironmentSwapper
        {
            EnvironmentSwapper(std::shared_ptr<Environment>& env, std::shared_ptr<Environment> newEnv) : env(env) { oldEnv = env; env = newEnv; }
            ~EnvironmentSwapper() { env = oldEnv; }

            std::shared_ptr<Environment> oldEnv;
            std::shared_ptr<Environment>& env;
        };
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(BinaryExpression *expr)
    {
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
        auto value = expr->expr->accept(this)->getElement();

        switch(expr->op.type)
        {
            case TokenType::INCREMENT:
                try
                {
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
        auto baseValue = expr->expr->accept(this);
        auto value = baseValue->getElement();

        switch(expr->op.type)
        {
            case TokenType::INCREMENT:
                try
                {
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
        return expr->expr->accept(this);
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(Atom *expr)
    {
        return std::make_shared<Variable>(expr->value);
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(List *expr)
    {
        auto currentEnv = std::make_shared<Environment>(env);
        EnvironmentSwapper swapper(env, currentEnv);

        std::queue<ExprPtr> emptyQueue;

        auto token = expr->squareBracket;
        token.text = "List Function";

        auto get = std::make_shared<AtFunction>(expr->self, currentEnv, "List Get", token, this, emptyQueue, nullptr);
        auto set = std::make_shared<SetFunction>(expr->self, currentEnv, "List Set", token, this, emptyQueue, nullptr);

        auto instance = std::make_shared<ClassInstance>(nullptr);

        std::unordered_map<std::string, std::shared_ptr<Function>> methods;

        methods["at"] = get->bind(instance);
        methods["set"] = set->bind(instance);

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
        return lookUpVariable(expr->name.text, expr->name, expr);
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(Var *expr)
    {    
        return lookUpVariable(expr->name.text, expr->name, expr);
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(Assign *expr)
    {
        auto value = expr->expr->accept(this);

        assignVariable(expr->name, expr, value);

        return value;
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(CallExpression *expr)
    {
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
        auto object = expr->object->accept(this);

        if(object->getType() == "ClassInstance")
            return std::static_pointer_cast<ClassInstance>(object)->get(expr->name);

        throw RuntimeException(expr->name, "Only instance have properties");
    }

    std::shared_ptr<Valuable> VisitorInterpreter::visit(Set *expr)
    {
        auto object = expr->object->accept(this);

        if(object->getType() != "ClassInstance")
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
        env->declareValue(stmt->name.text, std::make_shared<Function>(env, stmt->name.text, stmt->name, this, stmt->parameters, stmt->body));
    }

    void VisitorInterpreter::visitStatement(ClassStatement *stmt)
    {
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
        executeBlock(stmt->statements, this, std::make_shared<Environment>(env));
    }

    void VisitorInterpreter::visitStatement(IfStatement *stmt)
    {
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
        while(not returnTriggered and stmt->condition->accept(this)->getElement().isTrue())
        {
            stmt->body->accept(this);
        }
    }

    void VisitorInterpreter::visitStatement(ReturnStatement *stmt)
    {
        if(stmt->value)
            returnValue = stmt->value->accept(this);

        returnTriggered = true;
    }

    void VisitorInterpreter::visitStatement(ImportStatement *stmt)
    {
        
    }

    std::shared_ptr<Environment> VisitorInterpreter::ancestor(int distance) const
    {
        auto currentEnv = env;
        
        for(int i = 0; i < distance; i++)
            currentEnv = currentEnv->getEnv();

        return currentEnv;
    }

    std::shared_ptr<Valuable> VisitorInterpreter::lookUpVariable(const std::string& name, const Token& token, Expression* expression) const
    {
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
        return ancestor(distance)->getValue(name, token);
    }

    void VisitorInterpreter::assignVariable(const Token& name, Expression* expression, std::shared_ptr<Valuable> value)
    {
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
        ancestor(distance)->assignValue(name.text, name, value);
    }

    std::shared_ptr<Valuable> executeBlock(std::queue<StatementPtr> statements, VisitorInterpreter* visitor, std::shared_ptr<Environment> environment)
    {
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

}
