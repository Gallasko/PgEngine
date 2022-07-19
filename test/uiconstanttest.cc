#include "gtest/gtest.h"

#include "UI/uiconstant.h"

#include <iostream>

namespace pg
{
    namespace test
    {
        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------

        TEST(ui_value_test, initialization)
        {
            UiSize value;

            EXPECT_EQ(value, 0.0f);
        }

        TEST(ui_value_test, assignment)
        {
            UiSize value;

            EXPECT_EQ(value, 0.0f);

            value = 1;

            EXPECT_EQ(value, 1.0f);

            value = 23.04f;

            EXPECT_EQ(value, 23.04f);

            UiSize value2 = -15.02f;

            value = value2;

            EXPECT_EQ(value, -15.02f);

            UiSize value3 = 2.2f;

            value = &value3;

            EXPECT_EQ(value, 2.2f);

            value3 = 12;

            EXPECT_EQ(value, 12.0f);
        }

        TEST(ui_value_test, basic_operation)
        {
            UiSize value = 1.0f;

            EXPECT_EQ(value + 10.0f, 11.0f);
            EXPECT_EQ(value - 10.0f, -9.0f);
            EXPECT_EQ(value * 5.0f,  5.0f);
            EXPECT_EQ(value / 2.0f,  0.5f);
        }

        TEST(ui_value_test, assignment_operations)
        {
            UiSize value1 = 1.2f;
            UiSize value2 = 2.5f;

            UiSize res = value1 + value2;

            EXPECT_EQ(res, 3.7f);

            value1 = 1.8f;
            value2 = 3.1f;

            EXPECT_EQ(res, 4.9f);

            res = value1 - value2;

            EXPECT_EQ(res, -1.3f);

            value1 = 5.0f;
            value2 = 2.3f;

            EXPECT_EQ(res, 2.7f);

            res = value1 * value2;

            EXPECT_EQ(res, 11.5f);

            value1 = 2.0f;
            value2 = 3.0f;

            EXPECT_EQ(res, 6.0f);

            res = value1 / value2;

            EXPECT_EQ(res, 2.0f / 3.0f);

            value1 = 8.0f;
            value2 = 2.0f;

            EXPECT_EQ(res, 4.0f);

            EXPECT_EQ(-value1, -8.0f);
        }

    }
}