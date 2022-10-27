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

            EXPECT_EQ(logger.getNbError(), 0);

            interpreter.interpretFromText(testScript2);

            EXPECT_EQ(logger.getNbError(), 0);
        }

    } // namespace test

} // namespace pg