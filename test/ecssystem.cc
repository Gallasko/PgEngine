#include "gtest/gtest.h"

#include "ECS/uniqueid.h"
#include "ECS/entity.h"
#include "ECS/component.h"
#include "ECS/system.h"
#include "ECS/componentregistry.h"
#include "ECS/entitysystem2.h"

#include "mocklogger.h"

#include <iostream>
#include <string>

#include <chrono>

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
                C(ecs::_unique_id value, const std::string& text) : ecs::NamedComponent<C>("C"), value(value), text(text) {}

                ecs::_unique_id value;
                std::string text;
            };

            struct CSystem : public ecs::System<ecs::Own<C>>
            {
                CSystem(const size_t count) : ecs::System<ecs::Own<C>>(), count(count) {}

                virtual void execute()
                {
                    // for(auto comp : view<C>())
                        // std::cout << comp->id << " " << comp->text << std::endl;

                    auto list = view<C>();
                    auto it = list.begin();
                    for(size_t i = 0; it != list.end(); it++, i++)
                    {
                        //  if(i % count == 0)
                            // std::cout << (*it)->text << std::endl;
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
            constexpr size_t nbComps = 1000;

            MockLogger logger;
            // logger.addFilter("Log Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));

            auto start = std::chrono::steady_clock::now();

            ecs::EntitySystem ecs;

            auto end = std::chrono::steady_clock::now();

            std::cout << "Ecs creation took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            start = std::chrono::steady_clock::now();
            auto system = ecs.createSystem<ASystem>();
            end = std::chrono::steady_clock::now();

            std::cout << "System A creation took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            auto system2 = ecs.createSystem<ABSystem>();
            auto system3 = ecs.createSystem<CSystem>(nbComps / 50);

            std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            auto entity1 = ecs.createEntity();
            auto entity2 = ecs.createEntity();
            auto entity3 = ecs.createEntity();

            auto comp = system->createComponent<A>(entity1, 2, 5);
            auto comp1 = system2->createRefferedComponent<A>(entity2, 10, 5);
            auto comp2 = system2->createOwnedComponent<B>(entity3, 12, 4);

            std::cout << "Creating entities..." << std::endl;

            ecs::Entity *entity = new ecs::Entity[nbComps + 1];
            
            start = std::chrono::steady_clock::now();
            for(size_t i = 0; i < nbComps + 1; i++)
            {
                std::cout << "Creating entity: " << i << std::endl;
                entity[i] = ecs.createEntity();
                system->createOwnedComponent<A>(entity[i], i, 15);
                std::cout << "Created entity: " << i << std::endl;
            }
            end = std::chrono::steady_clock::now();

            std::cout << "Creating " << nbComps - 19 << " entities, and adding A took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            start = std::chrono::steady_clock::now();
            for(size_t i = 0; i < nbComps + 1; i++)
            {
                //ecs.attach<C>(entity.id, "Value of: " + std::to_string(i));
                system3->createOwnedComponent<C>(entity[i], entity[i].id, "Value of: " + std::to_string(i));
            }
            end = std::chrono::steady_clock::now();

            std::cout << "Adding C to " << nbComps + 1 << " entities took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            std::cout << "Entity " << entity[555].id << " has: [";
            for (auto& comp : entity[555].componentList)
            {
                std::cout << std::to_string(comp.first) << ", ";
            }

            std::cout << "]" << std::endl;

            start = std::chrono::steady_clock::now();
            system3->execute();
            end = std::chrono::steady_clock::now();

            std::cout << "System C execution took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            start = std::chrono::steady_clock::now();
            system3->execute();
            end = std::chrono::steady_clock::now();

            std::cout << "System C execution took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            start = std::chrono::steady_clock::now();
            system3->execute();
            end = std::chrono::steady_clock::now();

            std::cout << "System C execution took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            start = std::chrono::steady_clock::now();
            system3->execute();
            end = std::chrono::steady_clock::now();

            std::cout << "System C execution took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            std::cout << "End" << std::endl;

            delete[] entity;
        }

        TEST(system_test, group)
        {
            // MockLogger logger;
            // logger.addFilter("Log Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));

            constexpr size_t nbComps = 10000000;

            std::cout << "Number of entities: " << nbComps << std::endl;

            // MockLogger logger;

            auto start = std::chrono::steady_clock::now();

            ecs::EntitySystem ecs;

            auto end = std::chrono::steady_clock::now();

            std::cout << "Ecs creation took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            auto asys  = ecs.createSystem<ASystem>();
            
            auto absys = ecs.createSystem<ABSystem>();

            auto entity = new ecs::Entity[nbComps];

            for(size_t i = 0; i < nbComps; i++)
            {
                entity[i] = ecs.createEntity();
                asys->createComponent<A>(entity[i], i, 5);

                if(i % 2 == 0)
                    absys->createOwnedComponent<B>(entity[i], i, 5);
            }

            start = std::chrono::steady_clock::now();

            auto group = absys->group<A, B>();

            end = std::chrono::steady_clock::now();

            // MockLogger logger;

            std::cout << "Grouping [" << group->elements.nbElements() << "] entities took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            start = std::chrono::steady_clock::now();

            for(const auto& element : group->elements.viewComponents())
            {
                // std::cout << "Showing element: " << element->entityId << std::endl;
                // if(!element->toBeDeleted)
                element->get<A>()->value++;
                // std::cout << element->entityId << std::endl;
                // std::cout << element->get<A>()->value << std::endl;
                // std::cout << element->get<B>()->value << std::endl;
                
                // element.get<A>();
                // element.get<B>();
            }

            end = std::chrono::steady_clock::now();

            std::cout << "Iteration on group took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

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