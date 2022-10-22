#include <iostream>

#include "lexer.h"
#include "parser.h"
#include "resolver.h"
#include "interpreter.h"

#include "logger.h"

#include "systemfunction.h"

namespace
{
    const char * DOM = "Main";
}

int main(int argc, char **argv)
{
    std::string filename = "res/test8";

    if(argc >= 2)
        filename = argv[1];

    auto terminalSink = pg::Logger::registerSink<pg::TerminalSink>(true);
    terminalSink->addFilter("Log Level Filter", new pg::Logger::LogSink::FilterLogLevel(pg::Logger::InfoLevel::log));
    
    Lexer lexer;

    try
    {
        lexer.readFile(filename.c_str());
    }
    catch(const std::exception& e)
    {
        LOG_ERROR(DOM, e.what());
        return 45;
    }
    
    auto tokens = lexer.getTokens();

    Parser parser(tokens);

    auto statements = parser.parse();

    Resolver resolver(statements);

    auto localsList = resolver.resolve();

    if(parser.hasError())
    {
        LOG_ERROR(DOM, "Parser error");
        return 50;
    }

    if(resolver.hasError())
    {
        LOG_ERROR(DOM, "Resolver error");
        return 55;
    }

    for(auto local : localsList)
        std::cout << local.first->getName() << " " << local.first << " requires " << local.second << " env of lookup" << std::endl;

    Interpreter interpreter(resolver.getStatementsList(), localsList);

    interpreter.defineSystemFunction<ToString>("toString");
    interpreter.defineSystemFunction<LogInfo>("logInfo");
    interpreter.defineSystemFunction<HRClock>("mTime");

    interpreter.interpret();

    if(interpreter.hasError())
    {
        LOG_ERROR(DOM, "Interpreter error");
        return 60;
    }

    return 0;
}