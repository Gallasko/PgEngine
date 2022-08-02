#include "gtest/gtest.h"

#include "ECS/sparset.h"

namespace pg
{
    namespace test
    {
        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(sparse_test, initialization)
        {
            ecs::SparseSet set;
            
            set.insert(1);
        }
    }
}
