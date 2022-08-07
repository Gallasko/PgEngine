#include "gtest/gtest.h"

#include "ECS/component.h"
#include "ECS/system.h"
#include "ECS/uniqueid.h"
#include "ECS/componentregistry.h"
#include "ECS/entitysystem2.h"

#include <iostream>
#include <string>

namespace pg
{
    namespace test
    {
        namespace
        {
            struct A : public ecs::NamedComponent<A>
            {
                A(int arg1, int arg2) : ecs::NamedComponent<A>("A")
                {
                    value = arg1 + arg2;
                }

                int value;
            };

            struct B : public ecs::NamedComponent<B>
            {
                B(int arg1, int arg2) : ecs::NamedComponent<B>("B")
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

            struct C : public ecs::NamedComponent<C>
            {
                C(const std::string& text) : ecs::NamedComponent<C>("C"), text(text) {}

                std::string text;
            };

            struct CSystem : public ecs::System<ecs::Own<C>>
            {
                CSystem(const size_t count) : ecs::System<ecs::Own<C>>(), count(count) {}

                virtual void execute()
                {
                    auto list = view<C>();
                    auto it = list.begin();
                    for(size_t i = 0; it != list.end(); it++, i++)
                    {
                        if(i % count == 0)
                            std::cout << (*it)->text << std::endl;
                    }
                }

                size_t count = 1;
            };

            struct D : ecs::Component<D>
            {
                std::string text;
            };

/*
            struct CDSystem : public ecs::System<ecs::Ref<C>, ecs::Own<D>>
            {
                virtual void execute() override
                {
                    std::cout << "Execute CD System" << std::endl;

                    for(auto entity : view<C, D>())
                    {

                    }
                }
            }
*/
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(system_test, initialization)
        {
            ecs::EntitySystem ecs;

            std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            auto system = ecs.createSystem<ASystem>();

            std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            auto system2 = ecs.createSystem<ABSystem>();
            auto system3 = ecs.createSystem<CSystem>(1000000);

            std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            auto entity1 = ecs.createEntity();
            auto entity2 = ecs.createEntity();
            auto entity3 = ecs.createEntity();

            auto comp = system->createComponent<A>(entity1, 2, 5);
            auto comp1 = system2->createRefferedComponent<A>(entity2, 10, 5);
            auto comp2 = system2->createOwnedComponent<B>(entity3, 12, 4);

            std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            auto entity = ecs.createEntity();

            for(size_t i = 20; i < 1000000; i++)
            {
                system->createOwnedComponent<A>(entity, i, 15);
            }

            for(size_t i = 20; i < 10000001; i++)
            {
                //ecs.attach<C>(entity.id, "Value of: " + std::to_string(i));
                system3->createOwnedComponent<C>(entity, "Value of: " + std::to_string(i));
            }

            system3->execute();

            std::cout << "End" << std::endl;
        }

        /*
        TEST(system_test, system_perf_owned)
        {
            ecs::EntitySystem ecs;

            auto system = ecs.createSystem<ASystem>();

            auto entity = ecs.createEntity();

            for(size_t i = 20; i < 10000000; i++)
            {
                system->createOwnedComponent<A>(entity.id, i, 15);
            }
        }

        
        TEST(system_test, system_perf_create)
        {
            ecs::EntitySystem ecs;

            auto system = ecs.createSystem<ASystem>();

            auto entity = ecs.createEntity();

            for(size_t i = 20; i < 10000000; i++)
            {
                system->createComponent<A>(entity.id, i, 15);
            }
        }

        TEST(system_test, system_perf_attach)
        {
            ecs::EntitySystem ecs;

            auto system = ecs.createSystem<ASystem>();

            auto entity = ecs.createEntity();

            for(size_t i = 20; i < 10000000; i++)
            {
                ecs.attach<A>(entity, i, 15);
            }
        }
        */
    }
}