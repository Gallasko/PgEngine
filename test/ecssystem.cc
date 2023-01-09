#include "gtest/gtest.h"

#include "ECS/uniqueid.h"
#include "ECS/entity.h"
#include "ECS/system.h"
#include "ECS/componentregistry.h"
#include "ECS/entitysystem.h"

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
            struct A
            {
                A(int arg1, int arg2)
                {
                    value = arg1 + arg2;
                }

                int value;
            };

            struct B
            {
                B(int arg1, int arg2)
                {
                    value = arg1 - arg2;
                }

                int value;
            };

            struct ASystem : public System<Own<A>>
            {                
                virtual void execute() { std::cout << "Execute A System" << std::endl; }
            };

            struct ABSystem : public System<Ref<A>, Own<B>>
            {
                virtual void execute() { std::cout << "Execute B System" << std::endl; }
            };

            struct C
            {
                C(_unique_id value, const std::string& text) : value(value), text(text) {}

                _unique_id value;
                std::string text;
            };

            struct CSystem : public System<Own<C>>
            {
                CSystem(const size_t count) : System<Own<C>>(), count(count) {}

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

            struct D
            {
                std::string text;
            };

            struct DSystem : public System<Own<D>>
            {                
                virtual void execute() { std::cout << "Execute D System" << std::endl; }
            };

            struct EEvent
            {
                std::string payload;
            };

            struct E
            {
                std::string message;
            };

            struct ESystem : public System<Own<E>, Listener<EEvent>, StoragePolicy>
            {
                virtual void execute() { LOG_INFO("Event", "Execute E System"); }

                virtual void onEvent(const EEvent& e) { LOG_TEST("Event", e.payload); }
            };

            struct CreateDummy
            {
                int i;
            };

            struct CreateSystem : public System<Own<CreateDummy>>
            {
                CreateSystem(EntitySystem *ecs) : ecsRef(ecs) {}

                virtual void execute()
                {
                    LOG_INFO("Event", "Execute CreateSystem");

                    auto entity = ecsRef->createEntity();

                    LOG_TEST("Create System", Strfy() << "Created entity " << entity->id);
                }

                EntitySystem *ecsRef;
            };

/*
            struct CDSystem : public System<Ref<C>, Own<D>>
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
        TEST(component_registry_test, initialization)
        {
            ASystem  sys1;
            ABSystem sys2;
            CSystem  sys3(1);

            ComponentRegistry reg;

            sys1.addToRegistry(&reg);
            sys2.addToRegistry(&reg);
            sys3.addToRegistry(&reg);

            EXPECT_EQ(reg.getTypeId<A>(), 4);
            EXPECT_EQ(reg.getTypeId<B>(), 6);
            EXPECT_EQ(reg.getTypeId<C>(), 8);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(component_registry_test, multiple_registy)
        {
            // MockLogger logger;

            ASystem  sys1;
            ABSystem sys2; // AbSystem sys2bis // Todo here we get a cryptic map_base::at thrown exception when it is cause by A not being own by anything when creating AB <-- make a better compiler error ! or even catch it
            CSystem  sys3(1), sys3bis(1);
            DSystem  sys4;

            ComponentRegistry reg, reg2;

            sys1.addToRegistry(&reg);
            sys2.addToRegistry(&reg);
            sys3.addToRegistry(&reg);

            sys3bis.addToRegistry(&reg2);
            sys4.addToRegistry(&reg2);

            EXPECT_EQ(reg.getTypeId<A>(), 4);
            EXPECT_EQ(reg.getTypeId<B>(), 6);
            EXPECT_EQ(reg.getTypeId<C>(), 8);

            EXPECT_EQ(reg2.getTypeId<C>(), 4);
            EXPECT_EQ(reg2.getTypeId<D>(), 6);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(system_test, initialization)
        {
            constexpr size_t nbComps = 1000;

            MockLogger<FileSink> logger;
            // logger.addFilter("Log Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));

            auto start = std::chrono::steady_clock::now();

            EntitySystem ecs;

            auto end = std::chrono::steady_clock::now();

            std::cout << "Ecs creation took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            // Todo make a test with that to verify that component id are not shared between ECS
            // To get an id of a component you need to interact with the registry
            // std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            start = std::chrono::steady_clock::now();
            auto system = ecs.createSystem<ASystem>();
            end = std::chrono::steady_clock::now();

            std::cout << "System A creation took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            // std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            auto system2 = ecs.createSystem<ABSystem>();
            auto system3 = ecs.createSystem<CSystem>(nbComps / 50);

            // std::cout << A::componentId << " " << B::componentId << " " << C::componentId << std::endl;

            auto entity1 = ecs.createEntity();
            auto entity2 = ecs.createEntity();
            auto entity3 = ecs.createEntity();

            auto comp = system->createComponent<A>(entity1, 2, 5);
            auto comp1 = system2->createRefferedComponent<A>(entity2, 10, 5);
            auto comp2 = system2->createOwnedComponent<B>(entity3, 12, 4);

            std::cout << "Creating entities..." << std::endl;

            Entity **entity = new Entity*[nbComps + 1];
            
            start = std::chrono::steady_clock::now();
            for(size_t i = 0; i < nbComps + 1; i++)
            {
                // std::cout << "Creating entity: " << i << std::endl;
                entity[i] = ecs.createEntity();
                system->createOwnedComponent<A>(entity[i], i, 15);
                // std::cout << "Created entity: " << i << std::endl;
            }
            end = std::chrono::steady_clock::now();

            std::cout << "Creating " << nbComps - 19 << " entities, and adding A took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            start = std::chrono::steady_clock::now();
            for(size_t i = 0; i < nbComps + 1; i++)
            {
                //ecs.attach<C>(entity.id, "Value of: " + std::to_string(i));
                system3->createOwnedComponent<C>(entity[i], entity[i]->id, "Value of: " + std::to_string(i));
            }
            end = std::chrono::steady_clock::now();

            std::cout << "Adding C to " << nbComps + 1 << " entities took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            std::cout << "Entity " << entity[555]->id << " has: [";
            for (auto& comp : entity[555]->componentList)
            {
                std::cout << std::to_string(comp.getId()) << ", ";
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

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(system_test, basic_event_handling)
        {
            MockLogger logger;

            EntitySystem ecs;

            auto sys = ecs.createSystem<ESystem>();

            EEvent event {"New E event !"};

            ecs.sendEvent(event);

            EXPECT_EQ(logger.getNbTest(),  1);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(system_test, entity_creation_during_runtime)
        {
            MockLogger<TerminalSink> logger;

            EntitySystem ecs;

            auto sys = ecs.createSystem<CreateSystem>(&ecs);

            EXPECT_EQ(ecs.getNbEntities(),  0);

            CreateDummy dummy {1};

            ecs.executeAll();

            EXPECT_EQ(ecs.getNbEntities(),  0);

            ecs.executeAll();

            EXPECT_EQ(ecs.getNbEntities(),  1);

            // ecs.sendEvent(event);

            // EXPECT_EQ(logger.getNbTest(),  1);
        }

/*
        TEST(system_test, group)
        {
            // MockLogger logger;
            // logger.addFilter("Log Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));
            // logger.addFilter("Mile Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::mile));

            constexpr size_t nbComps = 10000000;

            std::cout << "Number of entities: " << nbComps << std::endl;

            // MockLogger logger;

            auto start = std::chrono::steady_clock::now();

            EntitySystem ecs;

            auto end = std::chrono::steady_clock::now();

            std::cout << "Ecs creation took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            auto asys  = ecs.createSystem<ASystem>();
            
            auto absys = ecs.createSystem<ABSystem>();

            auto entity = new Entity*[nbComps];

            for(size_t i = 0; i < nbComps; i++)
            {
                entity[i] = ecs.createEntity();
                asys->createComponent<A>(entity[i], i, 5);

                if(i % 2 == 0)
                    absys->createOwnedComponent<B>(entity[i], i, 5);
            }

            MockLogger logger;

            start = std::chrono::steady_clock::now();

            auto group = absys->group<A, B>();

            end = std::chrono::steady_clock::now();

            // MockLogger logger;

            std::cout << "Grouping [" << group->elements.nbElements() << "] entities took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

            start = std::chrono::steady_clock::now();

            {
                parallelFor(group->elements.nbElements(), [&group](size_t start, size_t end) { 
                    for(size_t i = start; i < end; i++)
                    {
                        // std::cout << "Working on element: " << i << std::endl;
                        if(i == 0) continue;

                        const auto& element = group->elements[i];
                        // std::cout << "Showing element: " << element->entityId << std::endl;
                        // if(!element->toBeDeleted)
                        element->get<A>()->value++;
                        // std0::cout << element->entityId << std::endl;
                        // std::cout << element->get<A>()->value << std::endl;
                        // std::cout << element->get<B>()->value << std::endl;
                        
                        // element.get<A>();
                        // element.get<B>();

                        // nbTestElements++;
                    }
                });
            }

            end = std::chrono::steady_clock::now();

            std::cout << "Iteration on group took: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns" << std::endl;

        }
*/

        /*
        TEST(system_test, system_perf_owned)
        {
            EntitySystem ecs;

            auto system = ecs.createSystem<ASystem>();

            auto entity = ecs.createEntity();

            for(size_t i = 20; i < 10000000; i++)
            {
                system->createOwnedComponent<A>(entity.id, i, 15);
            }
        }

        
        TEST(system_test, system_perf_create)
        {
            EntitySystem ecs;

            auto system = ecs.createSystem<ASystem>();

            auto entity = ecs.createEntity();

            for(size_t i = 20; i < 10000000; i++)
            {
                system->createComponent<A>(entity.id, i, 15);
            }
        }

        TEST(system_test, system_perf_attach)
        {
            EntitySystem ecs;

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