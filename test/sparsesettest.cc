#include <iostream>

#include "gtest/gtest.h"

#include "ECS/sparseset.h"
#include "ECS/entitysystem.h"

namespace pg
{
    namespace test
    {
        struct A
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

            SparseSet set;
        }

        TEST(sparse_test, iterate)
        {
            SparseSet set;

            for (int i = 1; i < 1000; i++)
            {
                set.add(i);
            }

            std::cout << set.nbElements() << std::endl;

            auto view = set.view();

            for(size_t i = 1; i < set.nbElements(); i++)
            {
                EXPECT_EQ(view[i], i);
            }
        }

    }
}
