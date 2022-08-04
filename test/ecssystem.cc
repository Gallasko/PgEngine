#include "gtest/gtest.h"

#include "ECS/component.h"
#include "ECS/system.h"
#include "uniqueid.h"

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
                A(int arg1, int arg2) : ecs::NamedComponent("A")
                {
                    value = arg1 + arg2;
                }

                int value;
            };

            struct B : public ecs::NamedComponent
            {
                B(int arg1, int arg2) : ecs::NamedComponent("B")
                {
                    value = arg1 - arg2;
                }

                int value;
            };

            struct ASystem : public ecs::System<ecs::Own<A>>
            {                
                virtual void execute() { std::cout << "Execute A System" << std::endl; }
            };

            struct ABSystem : public ecs::System<ecs::Ref<A>, ecs::Own<B>>
            {
                virtual void execute() { std::cout << "Execute B System" << std::endl; }
            };
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(system_test, initialization)
        {
            ASystem system;
            ABSystem system2;

            ecs::_entityId id = 15;
            ecs::_entityId id1 = 16;
            ecs::_entityId id2 = 17;

            auto comp = system.createOwnedComponent<A>(id, 2, 5);
            auto comp1 = system2.createRefferedComponent<A>(id1, 2, 5);
            auto comp2 = system2.createOwnedComponent<B>(id2, 2, 5);

            std::cout << "Value of comp: " << comp->value << " " << comp1->value << " " << comp2->value << std::endl; 
        }
    }
}