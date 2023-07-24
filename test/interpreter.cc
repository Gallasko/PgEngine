#include "gtest/gtest.h"

#include "mockinterpreter.h"

#include "mocklogger.h"

namespace pg
{
    namespace test
    {
        namespace
        {
            const char * testScript1 =  "var a = 1; var b = 2; var c = 3; \n" 
                                        "a = b + c                        \n" 
                                        "ExpectEq(a, 5);                  \n";

            const char * testScript2 =  "var a = 1; var b = 2; var c = 3; \n"
                                        "if (a == 1) ExpectEq(b, 2);      \n"
                                        "if (b < c)  ExpectEq(c, 3);      \n";
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, simple_script_opening_test)
        {
            MockLogger logger(false);

            MockInterpreter interpreter;

            interpreter.interpretFromFile("TestScripts/BasicOperation/addition.pg");

            EXPECT_EQ(logger.getNbError(), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, simple_script_from_file_test)
        {
            MockLogger logger(false);

            MockInterpreter interpreter;

            interpreter.interpretFromText(testScript1);

            EXPECT_EQ(logger.getNbError(), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, second_simple_script_from_file_test)
        {
            MockLogger logger(false);

            MockInterpreter interpreter;

            interpreter.interpretFromText(testScript2);

            EXPECT_EQ(logger.getNbError(), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, open_multiple_script_from_file_test)
        {
            MockLogger logger(false);

            MockInterpreter interpreter;

            interpreter.interpretFromText(testScript1);

            interpreter.interpretFromText(testScript2);

            EXPECT_EQ(logger.getNbError(), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, assignment_test)
        {
            MockLogger logger;
            MockInterpreter interpreter;

            auto scripts = UniversalFileAccessor::openTextFolder("TestScripts/Assignment/");

            for(auto script : scripts)
            {
                interpreter.interpretFromFile(script);
            }

            EXPECT_EQ(logger.getNbError(), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, basic_operation_test)
        {
            MockLogger logger;
            MockInterpreter interpreter;

            auto scripts = UniversalFileAccessor::openTextFolder("TestScripts/BasicOperation/");

            for(auto script : scripts)
            {
                interpreter.interpretFromFile(script);
            }

            EXPECT_EQ(logger.getNbError(), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, conditionnal_test)
        {
            MockLogger logger;
            MockInterpreter interpreter;

            auto scripts = UniversalFileAccessor::openTextFolder("TestScripts/Conditionnal/");

            for(auto script : scripts)
            {
                interpreter.interpretFromFile(script);
            }

            EXPECT_EQ(logger.getNbError(), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, functionnal_test)
        {
            MockLogger logger;
            MockInterpreter interpreter;

            auto scripts = UniversalFileAccessor::openTextFolder("TestScripts/Functionnal/");

            for(auto script : scripts)
            {
                interpreter.interpretFromFile(script);
            }

            EXPECT_EQ(logger.getNbError(), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(interpreter_test, table_test)
        {
            MockLogger logger(true);
            logger.addFilter("Log Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));

            MockInterpreter interpreter;

            auto scripts = UniversalFileAccessor::openTextFolder("TestScripts/Table/");

            for(auto script : scripts)
            {
                interpreter.interpretFromFile(script);
            }

            EXPECT_EQ(logger.getNbError(), 0);
        }

    } // namespace test

} // namespace pg