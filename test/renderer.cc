#include "gtest/gtest.h"

#include "Renderer/renderer.h"

namespace pg
{
    namespace test
    {
        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(render_call_test, initialization)
        {
            RenderCall call;

            EXPECT_EQ(call.key,          0);
            EXPECT_EQ(call.data.size(),  0);
            EXPECT_EQ(call.batchable,    true);
        }

        TEST(render_call_test, setting_key_values)
        {
            RenderCall call;

            EXPECT_EQ(call.key,          0);
            EXPECT_EQ(call.data.size(),  0);
            EXPECT_EQ(call.batchable,    true);

            call.setDepth(-5);

            EXPECT_EQ(call.key, ((uint64_t)(0b000000000000111111111111) - 5) << 30);

            auto depth = call.getDepth();

            EXPECT_EQ(depth, -5);

            call.setVisibility(true);

            EXPECT_EQ(call.key, (((uint64_t)(0b000000000000111111111111) - 5) << 30) + ((uint64_t)0b1 << 63));

            bool visible = call.getVisibility();

            EXPECT_EQ(visible, true);
        }

    } // namespace test
    
} // namespace pg