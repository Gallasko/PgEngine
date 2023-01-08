#include <gtest/gtest.h>

#include "mocklogger.h"

namespace pg
{
    namespace test
    {
        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(mock_logger_test, initialization)
        {
            ASSERT_NO_THROW(MockLogger logger);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(mock_logger_test, log_reception)
        {
            MockLogger logger;

            EXPECT_EQ(logger.getNbLog(), 0);
            EXPECT_EQ(logger.getLastMessage().message, "");
            EXPECT_EQ(logger.getLastMessage().level, Logger::InfoLevel::log);

            LOG_THIS("Test");

            EXPECT_EQ(logger.getNbLog(), 1);

            LOG_THIS("Test");
            LOG_THIS("Test");

            EXPECT_EQ(logger.getNbLog(), 3);
            EXPECT_EQ(logger.getLastMessage().message, "");
            EXPECT_EQ(logger.getLastMessage().level, Logger::InfoLevel::log);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(mock_logger_test, different_message_reception)
        {
            MockLogger logger;

            EXPECT_EQ(logger.getNbLog(),   0);
            EXPECT_EQ(logger.getNbInfo(),  0);
            EXPECT_EQ(logger.getNbError(), 0);
            EXPECT_EQ(logger.getNbTest(),  0);
            EXPECT_EQ(logger.getLastMessage().message, "");
            EXPECT_EQ(logger.getLastMessage().level, Logger::InfoLevel::log);

            LOG_THIS("Test");

            EXPECT_EQ(logger.getNbLog(),   1);
            EXPECT_EQ(logger.getNbInfo(),  0);
            EXPECT_EQ(logger.getNbError(), 0);
            EXPECT_EQ(logger.getNbTest(),  0);
            EXPECT_EQ(logger.getLastMessage().message, "");
            EXPECT_EQ(logger.getLastMessage().level, Logger::InfoLevel::log);

            LOG_THIS("Test");
            LOG_THIS("Test");
            LOG_INFO("Test", "First Info");

            EXPECT_EQ(logger.getNbLog(),   3);
            EXPECT_EQ(logger.getNbInfo(),  1);
            EXPECT_EQ(logger.getNbError(), 0);
            EXPECT_EQ(logger.getNbTest(),  0);
            EXPECT_EQ(logger.getLastMessage().message, "First Info");
            EXPECT_EQ(logger.getLastMessage().level, Logger::InfoLevel::info);

            LOG_THIS("Test");
            LOG_TEST("Test", "Test log");

            EXPECT_EQ(logger.getNbLog(),   4);
            EXPECT_EQ(logger.getNbInfo(),  1);
            EXPECT_EQ(logger.getNbError(), 0);
            EXPECT_EQ(logger.getNbTest(),  1);
            EXPECT_EQ(logger.getLastMessage().message, "Test log");
            EXPECT_EQ(logger.getLastMessage().level, Logger::InfoLevel::test);

            LOG_THIS("Test");

            EXPECT_EQ(logger.getNbLog(),   5);
            EXPECT_EQ(logger.getNbInfo(),  1);
            EXPECT_EQ(logger.getNbError(), 0);
            EXPECT_EQ(logger.getNbTest(),  1);
            EXPECT_EQ(logger.getLastMessage().message, "");
            EXPECT_EQ(logger.getLastMessage().level, Logger::InfoLevel::log);

            LOG_INFO("Test", "Second Info");
            LOG_THIS("Test");
            LOG_ERROR("Test", "First Error");

            EXPECT_EQ(logger.getNbLog(),   6);
            EXPECT_EQ(logger.getNbInfo(),  2);
            EXPECT_EQ(logger.getNbError(), 1);
            EXPECT_EQ(logger.getNbTest(),  1);
            EXPECT_EQ(logger.getLastMessage().message, "First Error");
            EXPECT_EQ(logger.getLastMessage().level, Logger::InfoLevel::error);
        } 

    } // namespace test

} // namespace pg