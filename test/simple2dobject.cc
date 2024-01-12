#include "gtest/gtest.h"

#include "2D/simple2dobject.h"

namespace pg
{
    namespace test
    {
        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(system_2d, initialization)
        {
            MasterRenderer renderer;

            Simple2DObjectSystem sys(&renderer);

            EXPECT_EQ(sys.currentSize,     1);
            EXPECT_EQ(sys.elementIndex,    0);
            EXPECT_EQ(sys.visibleElements, 0);
        }

        TEST(system_2d, creation)
        {
            EntitySystem ecs;

            MasterRenderer renderer;

            auto sys = ecs.createSystem<Simple2DObjectSystem>(&renderer);

            auto ent = ecs.createEntity();

            EXPECT_EQ(sys->currentSize,     1);
            EXPECT_EQ(sys->elementIndex,    0);
            EXPECT_EQ(sys->visibleElements, 0);

            ecs.attach<UiComponent>(ent);

            ecs.attach<Simple2DObject>(ent, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     2);
            EXPECT_EQ(sys->elementIndex,    1);
            EXPECT_EQ(sys->visibleElements, 1);
        }
    }
}