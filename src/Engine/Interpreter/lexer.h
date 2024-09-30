#pragma once

/**
 * @file lexer.h
 * @author Gallasko
 * @brief Definition of the lexer class
 * @version 1.0
 * @date 2022-04-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <string>
#include <queue>
#include <stdexcept>

#include "token.h"

namespace pg
{

    class LexerException : public std::runtime_error
    {
    public: 
        LexerException(const std::string& message, int line, int column) noexcept : std::runtime_error(createErrorMessage(message, line, column)) {}
        virtual ~LexerException() = default;

        std::string createErrorMessage(const std::string& message, int line, int column) const noexcept;
    };

    class Lexer
    {
    public:
        Lexer() { }

        void readFromText(const std::string& script);
        void readFromFile(const std::string& filename);

        const std::queue<Token>& getTokens() const { return tokens; }

    private:

        std::queue<Token> tokens;
    };

}
