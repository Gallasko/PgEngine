#include "gtest/gtest.h"

#include "ECS/component.h"
#include "ECS/system.h"

#include <iostream>
#include <string>

namespace pg
{
    namespace test
    {
        namespace
        {
            struct A : public ecs::NamedComponent
            {
                A(int arg1, int arg2) : ecs::NamedComponent("temp")
                {
                    value = arg1 + arg2;
                }

                int value;
            };

            struct B : public ecs::NamedComponent
            {
                B(int arg1, int arg2) : ecs::NamedComponent("temp")
                {
                    value = arg1 - arg2;
                }

                int value;
            };

            struct ASystem : public ecs::System<ecs::Own<A>, ecs::Own<B>>
            {                
                virtual void execute() { std::cout << "Execute A System" << std::endl; }
            };
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(system_test, initialization)
        {
            ASystem system;

            auto comp = system.createComponent<A>(2, 5);

            std::cout << "Value of comp: " << comp->value << std::endl; 
        }
    }
}