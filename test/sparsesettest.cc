#include <iostream>

#include "gtest/gtest.h"

#include "ECS/sparseset.h"
#include "ECS/entitysystem2.h"

namespace pg
{
    namespace test
    {
        struct A : public ecs::Component<A>
        {
            A(int data) : data(data) {}

            int data = 0;
        };

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(sparse_test, initialization)
        {
            std::cout << "Test initialization" << std::endl;

            ecs::SparseSet set;
        }

        TEST(sparse_test, iterate)
        {
            ecs::SparseSet set;
            ecs::EntitySystem ecs;

            for (int i = 1; i < 1000; i++)
            {
                auto entity = ecs.createEntity();
                set.add(entity, new A(i));
            }

            std::cout << set.nbElements() << std::endl;

            auto view = set.view<A>();

            for(size_t i = 1; i < set.nbElements(); i++)
            {
                EXPECT_EQ(view[i]->data, i);
            }
        }

    }
}
