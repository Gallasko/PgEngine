/**
 * @addtogroup cctv
 * @{
 * @addtogroup test
 * @{
 * @file
 * Tests to validate the worker
 *
 * @author ggs
 */
#include "gtest/gtest.h"

#include "UI/uisystem.h"

#include <iostream>

namespace pg
{
    namespace test
    {
        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(component_test, assert_initialization)
        {
            UiComponent component;
            
            EXPECT_EQ(component.pos.x, 0.0f);
            EXPECT_EQ(component.pos.y, 0.0f);
            EXPECT_EQ(component.pos.z, 0.0f);

            EXPECT_EQ(component.width, 0.0f);
            EXPECT_EQ(component.height, 0.0f);

        }

    } // namespace test
} // namespace pg

/**@}*/
/**@}*/
