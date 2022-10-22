#include "parser.h"

#include "logger.h"

namespace pg
{

    namespace
    {
        const char * DOM = "Parser";
    }

    std::string ParseException::createErrorMessage(const Token& token, const std::string& message) const noexcept
    {
        std::string errStr = "Parser Error: " + message + " at line " + std::to_string(token.line) + ", column " + std::to_string(token.column);
        
        return errStr;
    }

    std::queue<StatementPtr> Parser::parse()
    {
        std::queue<StatementPtr> statements;

        while(not isAtEnd())
        {
            try
            {
                statements.push(declaration());
            }
            catch (const ParseException& e)
            {
                //Synchronize
                synchronize();

                LOG_ERROR(DOM, e.what());
                errorEncountered = true;
            }
        }

        return statements;
    }

    std::queue<StatementPtr> Parser::block()
    {
        std::queue<StatementPtr> statements;

        while((not check(TokenType::BCLOSE)) and (not isAtEnd()))
        {
            statements.push(declaration());

            skipEOL();
        }

        consume("Expect a } at the end of a block", TokenType::BCLOSE);

        return statements;
    }

    ExprPtr Parser::finishCall(ExprPtr caller)
    {
        std::queue<ExprPtr> arguments;

        if(not check(TokenType::PCLOSE))
        {
            do
            {
                arguments.push(expression());
                skipEOL();
            } while(match(TokenType::COMMA));
        }

        skipEOL();
        Token paren = consume("Expect ')' after arguments", TokenType::PCLOSE);

        return std::make_shared<CallExpression>(caller, paren, arguments);
    }

    ExprPtr Parser::finishList()
    {
        int nbEntries = 0;
        std::queue<ListElement> entries;

        ExprPtr lExpr;
        ExprPtr rExpr;

        if(not check(TokenType::CCLOSE))
        {
            do
            {
                lExpr = expression();
                skipEOL();

                // If a double point is present then the user made a dict entry like so "key: value"
                if(match(TokenType::DPOINT))
                {
                    skipEOL();
                    rExpr = expression();
                    skipEOL();
                }
                // Else we set the key of the value as the number of entries in the dict like so "nbEntries: value"
                else
                {
                    rExpr = lExpr;
                    lExpr = std::make_shared<Atom>(nbEntries);
                }

                nbEntries++;

                entries.push(ListElement{lExpr, rExpr});
            } while(match(TokenType::COMMA));
        }

        skipEOL();
        Token squareBracket = consume("Expect ']' after arguments", TokenType::CCLOSE);

        auto self = std::make_shared<This>(Token{TokenType::EXPRESSION, "this", squareBracket.line, squareBracket.column});

        return std::make_shared<List>(self, squareBracket, entries);
    }

    ExprPtr Parser::expression()
    {
        return assignment();
    }

    ExprPtr Parser::assignment()
    {
        auto expr = logicOr();

        while(match(TokenType::EQUAL, TokenType::PLUSEQUAL, TokenType::MINUSEQUAL, TokenType::STAREQUAL, TokenType::DIVIDEQUAL))
        {
            Token op = previousToken;

            skipEOL();
            auto rExpr = assignment();

            // Desugaring operator +=, -=, *= and /=
            switch(op.type)
            {
            case TokenType::PLUSEQUAL:
                rExpr = std::make_shared<BinaryExpression>(expr, Token{TokenType::PLUS, "+", op.line, op.column}, rExpr);
                break;

            case TokenType::MINUSEQUAL:
                rExpr = std::make_shared<BinaryExpression>(expr, Token{TokenType::MINUS, "-", op.line, op.column}, rExpr);
                break;

            case TokenType::STAREQUAL:
                rExpr = std::make_shared<BinaryExpression>(expr, Token{TokenType::STAR, "*", op.line, op.column}, rExpr);
                break;

            case TokenType::DIVIDEQUAL:
                rExpr = std::make_shared<BinaryExpression>(expr, Token{TokenType::SLASH, "/", op.line, op.column}, rExpr);
                break;

            default:
                break;
            }

            // Check if the expression is a variable definition
            if(expr->getType() == "Var")
            {
                auto& token = std::static_pointer_cast<Var>(expr)->name;
                return std::make_shared<Assign>(token, rExpr);
            }
            // Check if the expression is a field from a class
            else if(expr->getType() == "Get")
            {
                auto getExpr = std::static_pointer_cast<Get>(expr);
                return std::make_shared<Set>(getExpr->object, getExpr->name, rExpr);
            }
            // Check if the expression is an assignment to an array
            else if(expr->getType() == "CallExpression" and expr->getName() == "at")
            {
                auto callExpr = std::static_pointer_cast<CallExpression>(expr);

                // Check if it is call to a get function and not a simple function call with a function named "at"
                if(callExpr->caller and callExpr->caller->getType() == "Get")
                {
                    auto getExpr = std::static_pointer_cast<Get>(callExpr->caller);

                    // Change the function call from "at" to "set"
                    getExpr->name.text = "set";
                    // Push the assign value as the second parameter of the set function call
                    callExpr->args.push(rExpr);
                    return expr;
                }
            }

            LOG_ERROR(DOM, "Syntax Error: Invalid lvalue for assignment at line " + std::to_string(op.line) + ", column " + std::to_string(op.column));
            errorEncountered = true;
        }

        return expr;
    }

    ExprPtr Parser::logicOr()
    {
        auto expr = logicAnd();

        while(match(TokenType::LOGICOR))
        {
            Token op = previousToken;
            
            skipEOL();
            auto rExpr = logicAnd();

            expr = std::make_shared<LogicExpression>(expr, op, rExpr);
        }

        return expr;
    }

    ExprPtr Parser::logicAnd()
    {
        auto expr = equality();

        while(match(TokenType::LOGICAND))
        {
            Token op = previousToken;
            
            skipEOL();
            auto rExpr = equality();

            expr = std::make_shared<LogicExpression>(expr, op, rExpr);
        }

        return expr;
    }

    ExprPtr Parser::equality()
    {
        auto expr = comparison();

        while(match(TokenType::NOTEQUAL, TokenType::EQUALEQUAL))
        {
            Token op = previousToken;
            
            skipEOL();
            auto rExpr = comparison();

            expr = std::make_shared<BinaryExpression>(expr, op, rExpr);
        }

        return expr;
    }

    ExprPtr Parser::comparison()
    {
        auto expr = term();

        while(match(TokenType::SUPEQUAL, TokenType::INFEQUAL, TokenType::SUP, TokenType::INF))
        {
            Token op = previousToken;
            
            skipEOL();
            auto rExpr = term();

            expr = std::make_shared<BinaryExpression>(expr, op, rExpr);
        }

        return expr;
    }

    ExprPtr Parser::term()
    {
        auto expr = factor();

        while(match(TokenType::MINUS, TokenType::PLUS))
        {
            Token op = previousToken;
            
            skipEOL();
            auto rExpr = factor();

            expr = std::make_shared<BinaryExpression>(expr, op, rExpr);
        }

        return expr;
    }

    ExprPtr Parser::factor()
    {
        auto expr = unary();

        while(match(TokenType::SLASH, TokenType::STAR))
        {
            Token op = previousToken;
            
            skipEOL();
            auto rExpr = unary();

            expr = std::make_shared<BinaryExpression>(expr, op, rExpr);
        }

        return expr;
    }

    ExprPtr Parser::unary()
    {
        if (match(TokenType::NOT, TokenType::MINUS)) 
        {
            Token op = previousToken;
            auto expr = unary();

            return std::make_shared<UnaryExpression>(expr, op);
        }

        return call();
    }

    ExprPtr Parser::call()
    {
        auto expr = primary();

        while(true)
        {
            if(match(TokenType::PENTER))
            {
                skipEOL();
                expr = finishCall(expr);
            }
            else if(match(TokenType::POINT))
            {
                skipEOL();
                auto token = consume("Expect property name after '.'.", TokenType::EXPRESSION);
                
                expr = std::make_shared<Get>(expr, token);
            }
            else if(match(TokenType::CENTER))
            {
                auto token = previousToken;

                expr = std::make_shared<Get>(expr, Token{TokenType::EXPRESSION, "at", token.line, token.column});

                skipEOL();
                std::queue<ExprPtr> argument;
                argument.push(expression());

                skipEOL();
                consume("Expect ']' after an Array subscript call.", TokenType::CCLOSE);

                expr = std::make_shared<CallExpression>(expr, token, argument);
            }
            else
            {
                break;
            }
        }

        return expr;
    }

    ExprPtr Parser::primary()
    {
        if(match(TokenType::TRUE)) return std::make_shared<Atom>(true);
        if(match(TokenType::FALSE)) return std::make_shared<Atom>(false);

        //TODO see if it is relevant to create a null expression
        //if(match(TokenType::NULL)) return std::make_shared<Atom>(nullptr)

        //TODO currently only support integer types add float types when implemented in lexer
        if(match(TokenType::NUMBER)) return std::make_shared<Atom>(std::stoi(previousToken.text));
        if(match(TokenType::FLOAT)) return std::make_shared<Atom>(std::stof(previousToken.text));
        if(match(TokenType::STRING)) return std::make_shared<Atom>(previousToken.text);

        if(match(TokenType::THIS)) return std::make_shared<This>(previousToken);

        if(match(TokenType::EXPRESSION)) return std::make_shared<Var>(previousToken);

        if (match(TokenType::PENTER)) {
            skipEOL();
            auto expr = expression();
            skipEOL();

            consume("Expect ')' after expression.", TokenType::PCLOSE);

            return std::make_shared<CompoundAtom>(expr);
        }

        if (match(TokenType::CENTER)) return finishList();

        throw ParseException(tokenList.front(), "Expected expression");
    }

    StatementPtr Parser::declaration()
    {
        try
        {
            if(match(TokenType::VAR))   return varDeclaration();
            if(match(TokenType::FUN))   return funDeclaration();
            if(match(TokenType::CLASS)) return classDeclaration();

            return statement();
        }
        catch(const std::exception& e)
        {
            synchronize();

            LOG_ERROR(DOM, e.what());
            errorEncountered = true;

            return nullptr;
        }
    }

    StatementPtr Parser::statement()
    {
        if(match(TokenType::RETURN)) return returnStatement();
        if(match(TokenType::FOR)) return forStatement();
        if(match(TokenType::IF)) return ifStatement();
        if(match(TokenType::WHILE)) return whileStatement();
        if(match(TokenType::BENTER)) return blockDeclaration();

        return expressionStatement();
    }

    std::shared_ptr<FunctionStatement> Parser::makeFun(const std::string& kind)
    {
        auto name = consume("Expected " + kind + " name", TokenType::EXPRESSION);

        skipEOL();
        consume("Expect '(' after a function declaration.", TokenType::PENTER);
        skipEOL();

        std::queue<ExprPtr> parameters;

        if(not check(TokenType::PCLOSE))
        {
            do
            {
                parameters.push(expression());
                skipEOL();
            } while(match(TokenType::COMMA));
        }

        skipEOL();
        Token paren = consume("Expect ')' after parameters", TokenType::PCLOSE);

        StatementPtr body = nullptr;

        if(match(TokenType::EOL))
        {
            skipEOL();

            if(match(TokenType::BENTER))
                body = blockDeclaration();
            else
                return std::make_shared<FunctionStatement>(name, parameters, body);
        }
        else
        {
            body = declaration();
        }

        consume("Expected ; or end of line after a variable definition", TokenType::END, TokenType::EOL);
        return std::make_shared<FunctionStatement>(name, parameters, body);
    }

    StatementPtr Parser::varDeclaration()
    {
        auto name = consume("Expected variable name", TokenType::EXPRESSION);

        ExprPtr initializer = nullptr;
        if(match(TokenType::EQUAL))
        {
            skipEOL();
            initializer = expression();
        }

        consume("Expected ; or end of line after a variable definition", TokenType::END, TokenType::EOL);
        return std::make_shared<VariableStatement>(name, initializer);
    }

    StatementPtr Parser::funDeclaration(const std::string& kind)
    {
        return makeFun(kind);
    }

    StatementPtr Parser::classDeclaration()
    {
        auto name = consume("Expected class name", TokenType::EXPRESSION);

        std::queue<std::shared_ptr<FunctionStatement>> body;

        if(match(TokenType::EOL))
        {
            skipEOL();

            if(not check(TokenType::BENTER))
                return std::make_shared<ClassStatement>(name, body);
        }

        consume("Expect '{' before class body", TokenType::BENTER);
        skipEOL();

        while(not check(TokenType::BCLOSE) && not isAtEnd())
        {
            body.push(makeFun("method"));
            skipEOL();
        }

        consume("Expect '}' after class body", TokenType::BCLOSE);

        return std::make_shared<ClassStatement>(name, body);
    }

    StatementPtr Parser::forStatement()
    {
        skipEOL();
        consume("Expect '(' after 'for'.", TokenType::PENTER);
        skipEOL();

        StatementPtr initializer;
        
        if(match(TokenType::END))
            initializer = nullptr;
        else if(match(TokenType::VAR))
            initializer = varDeclaration();
        else
            initializer = expressionStatement();

        skipEOL();
        ExprPtr condition = nullptr;
        skipEOL();

        if(not check(TokenType::END))
        {
            condition = expression();
            skipEOL();
        }

        consume("Expect ';' after loop condition.", TokenType::END);
        skipEOL();

        ExprPtr increment = nullptr;
        skipEOL();

        if(not check(TokenType::PCLOSE))
        {
            increment = expression();
            skipEOL();
        }

        consume("Expect ')' after 'for'.", TokenType::PCLOSE);
        skipEOL();

        // Desugaring of 'for' into a basic while loop

        StatementPtr body = statement();

        if(increment)
        {
            std::queue<StatementPtr> q;
            q.push(body);
            q.push(std::make_shared<ExpressionStatement>(increment));

            body = std::make_shared<BlockStatement>(q);
        }

        if(condition == nullptr)
            condition = std::make_shared<Atom>(true);

        body = std::make_shared<WhileStatement>(condition, body);

        if(initializer)
        {
            std::queue<StatementPtr> q;
            q.push(initializer);
            q.push(body);

            body = std::make_shared<BlockStatement>(q);
        }
            
        return body;
    }

    StatementPtr Parser::ifStatement()
    {
        skipEOL();
        consume("Expect '(' after 'if'.", TokenType::PENTER);
        skipEOL();

        ExprPtr condition = expression();

        skipEOL();
        consume("Expect ')' after 'if'.", TokenType::PCLOSE);
        skipEOL();
        
        StatementPtr thenBranch = statement();
        StatementPtr elseBranch = nullptr;

        skipEOL();

        if(match(TokenType::ELSE))
            elseBranch = statement();

        return std::make_shared<IfStatement>(condition, thenBranch, elseBranch);
    }

    StatementPtr Parser::whileStatement()
    {
        skipEOL();
        consume("Expect '(' after 'if'.", TokenType::PENTER);
        skipEOL();

        ExprPtr condition = expression();

        skipEOL();
        consume("Expect ')' after 'if'.", TokenType::PCLOSE);
        skipEOL();

        StatementPtr body = statement();

        return std::make_shared<WhileStatement>(condition, body);
    }

    StatementPtr Parser::returnStatement()
    {
        auto token = previousToken;

        ExprPtr returnValue = expression();

        consume("Expected ; or end of line after a return statement.", TokenType::END, TokenType::EOL);
        return std::make_shared<ReturnStatement>(token, returnValue);
    }

    StatementPtr Parser::blockDeclaration()
    {
        return std::make_shared<BlockStatement>(block());
    }

    StatementPtr Parser::expressionStatement()
    {
        if(isAtEnd())
            return nullptr;

        if(match(TokenType::END, TokenType::EOL))
            return declaration();

        ExprPtr expr = expression();

        consume("Expect a ; or an end of line at the end of an expression", TokenType::END, TokenType::EOL);
        return std::make_shared<ExpressionStatement>(expr);
    }

}
