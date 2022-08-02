#include <iostream>

#include "gtest/gtest.h"

#include "ECS/sparseset.h"

namespace pg
{
    namespace test
    {
        struct A : public ecs::Component
        {
            A(int data) : ecs::Component(ecs::tag<A>{}), data(data) {}

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
            /*
            ecs::SparseSet set;

            for (int i = 1; i < 1000; i++)
            {
                set.add(i, new A(i));
            }

            std::cout << set.nbElements() << std::endl;

            for(auto component : set.view<A>())
            {
                std::cout << component->data << std::endl;
            }
            */
        }

    }
}
