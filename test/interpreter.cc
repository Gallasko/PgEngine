#include "gtest/gtest.h"

#include "mocklogger.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    namespace test
    {
        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, simple_script_opening_test)
        {
            MockLogger logger;
            logger.addFilter("Log Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));

            PgInterpreter interpreter;

            interpreter.interpretFromFile("TestScripts/BasicOperation/addition.pg");

            EXPECT_EQ(logger.getNbInfo(),  1);

            EXPECT_EQ(logger.getLastMessage().message, "5");
        }

    } // namespace test

} // namespace pg