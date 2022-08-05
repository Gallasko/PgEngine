#include <gtest/gtest.h>

#include "ECS/entitysystem.h"

#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>

namespace pg
{
    namespace test
    {
        namespace
        {
            struct A
            {
                A() {}
                A(const std::string& name, int a) : name(name), a(a) {}
                
                std::string name;
                int a;
            };

            struct B
            {
                B() {}
                B(const std::string& name, int a, float b) : name(name), a(a), b(b) {}

                std::string name;
                int a;
                float b;
            };

            struct C
            {
                C() {}
                C(float x, float y, float z) : x(x), y(y), z(z) {}

                float x;
                float y;
                float z;
            };

            struct D
            {
                D(){}
                D(const std::string& patternName) : patternName(patternName) {}

                std::string patternName;
            };
        }

        TEST(ecs_test, test_lots_of_entities)
        {
            const unsigned int n = 1000;
            EntitySystem ecs;

            std::vector<EntitySystem::Entity*> entityList;
            entityList.reserve(n);

            for(unsigned int i = 0; i < n; i++)
            {
                auto entity = ecs.createEntity();
                ecs.attach<A>(entity, "Entity " + std::to_string(i), i );

                ecs.attach<B>(entity, "Entity " + std::to_string(i), i , i * 5.5f );

                //if(i % 2 == 0)
                //    ecs.attach<C>(entity, i, -i, 0 );
                //
                //if(i % 3 == 0)
                //    ecs.attach<D>(entity, "Guess i got pattern " + std::to_string(i % 2) );

                entityList[i] = entity;
            }

            for(unsigned int j = 0; j < n; j++)
            {
                ecs.removeEntity(entityList[j]);                
            }
        }

        TEST(ecs_test, permute)
        {
            constexpr int n = 4;

            EntitySystem ecs;

            auto groupA = ecs.registerGroup<A, B>();
            auto groupB = ecs.registerGroup<A, C>();
            auto groupC = ecs.registerGroup<A, D>();
            auto groupD = ecs.registerGroup<A, B, C>();
            auto groupE = ecs.registerGroup<A, C, D>();
            auto groupF = ecs.registerGroup<A, B, C, D>();

            EXPECT_EQ(groupA->size(), 0);
            EXPECT_EQ(groupB->size(), 0);
            EXPECT_EQ(groupC->size(), 0);
            EXPECT_EQ(groupD->size(), 0);
            EXPECT_EQ(groupE->size(), 0);
            EXPECT_EQ(groupF->size(), 0);

            int indice[n];
            
            for(int i = 0; i < n; i++)
                indice[i] = i;

            EntitySystem::Entity *entityList[n];

            do
            {
                for(auto i = 0; i < n; i++)
                {
                    auto entity = ecs.createEntity();
                    ecs.attach<A>(entity, "Entity " + std::to_string(i), i);

                    ecs.attach<B>(entity, "Entity " + std::to_string(i), i , i * 5.5f);

                    if(i % 2 == 0)
                        ecs.attach<C>(entity, i, -i, indice[i]);

                    if(i % 3 == 0)
                        ecs.attach<D>(entity, "Guess i got pattern " + std::to_string(i % 2));

                    entityList[i] = entity;
                }
                
                EXPECT_EQ(groupA->size(), n);
                EXPECT_EQ(groupB->size(), std::ceil(n / 2.0f));
                EXPECT_EQ(groupC->size(), std::ceil(n / 3.0f));
                EXPECT_EQ(groupD->size(), std::ceil(n / 2.0f));
                EXPECT_EQ(groupE->size(), std::ceil(n / 6.0f));
                EXPECT_EQ(groupF->size(), std::ceil(n / 6.0f));

                for(auto j = 0; j < n; j++)
                    ecs.removeEntity(entityList[indice[j]]);

                EXPECT_EQ(groupA->size(), 0);
                EXPECT_EQ(groupB->size(), 0);
                EXPECT_EQ(groupC->size(), 0);
                EXPECT_EQ(groupD->size(), 0);
                EXPECT_EQ(groupE->size(), 0);
                EXPECT_EQ(groupF->size(), 0);

            } while( std::next_permutation(indice, indice + n) );
        }

/**
        void testViews(int indices[], int n, bool debug = false)
        {
            EntitySystem ecs;

            std::vector<EntitySystem::Entity *> entityList;

            int nbA = 0, nbB = 0, nbPattern = 0, nbVector = 0;

            std::cout << "Testing set:";

            for(auto i = 0; i < n; i++)
                    std::cout << " " << indices[i];

            std::cout << std::endl; 

            for(auto i = 0; i < n; i++)
            {
                auto entity = ecs.createEntity();
                ecs.attach<A>(entity, { "Entity " + std::to_string(i), i } );
                nbA++;

                if(i % 2 == 0)
                {
                    ecs.attach<B>(entity, { "Entity " + std::to_string(i), i , i * 5.5f } );
                    nbB++;
                }
                    
                if(i % 3 == 0)
                {
                    ecs.attach<Pattern>(entity, { "Guess i got pattern " + std::to_string(i % 2) } );
                    nbPattern++;
                }
                    
                ecs.attach<Vector>(entity, { i, -i, 0 } );
                nbVector++;

                entityList.push_back(entity);

                if(i % 1000 == 999)
                    std::cout << "Created " << i << " Entities !" << std::endl;
            }

            for(auto j = 0; j < n; j++)
            {
                auto i = indices[j];
                ecs.removeEntity(entityList[i]);

                if(debug)
                    print3DContainer(ecs.view<Vector>(), "Printing every vector");

                nbA--;

                if(i % 2 == 0)
                    nbB--;
                    
                if(i % 3 == 0)
                    nbPattern--;

                nbVector--;

                //std::cout << nbA << " : " << viewLen(ecs.view<A>()) << std::endl;
                if(nbA != viewLen(ecs.view<A>()))
                {
                    std::cout << "View A mismatched" << std::endl;
                    throw(-1);
                }

                if(nbB != viewLen(ecs.view<B>()))
                {
                    std::cout << "View B mismatched" << std::endl;
                    throw(-1);
                }

                if(nbPattern != viewLen(ecs.view<Pattern>()))
                {
                    std::cout << "View Pattern mismatched" << std::endl;
                    throw(-1);
                }

                if(nbVector != viewLen(ecs.view<Vector>()))
                {
                    std::cout << "View Vector mismatched" << std::endl;
                    throw(-1);
                }

                if(j % 1000 == 999)
                    std::cout << "Deleted " << j << " Entities !" << std::endl;
                
            }

            print3DContainer(ecs.view<Vector>(), "Printing every vector");
        }

        void testGroups(int indices[], int n, bool debug = false)
        {
            EntitySystem ecs;

            std::vector<EntitySystem::Entity *> entityList;

            auto groupA = ecs.registerGroup<A, Vector>();
            auto groupB = ecs.registerGroup<A, B, Pattern, Vector>();
            auto groupC = ecs.registerGroup<B, Pattern>();
            auto groupD = ecs.registerGroup<Pattern, Vector>();
            auto groupE = ecs.registerGroup<Vector>();

            int nbA = 0, nbB = 0, nbPattern = 0, nbVector = 0, nbGroupB = 0;

            std::cout << "Testing set:";

            for(auto i = 0; i < n; i++)
                    std::cout << " " << indices[i];

            std::cout << std::endl; 

            for(auto i = 0; i < n; i++) 
            {
                auto entity = ecs.createEntity();
                ecs.attach<A>(entity, { "Entity " + std::to_string(i), i } );
                nbA++;

                if(i % 2 == 0)
                {
                    ecs.attach<B>(entity, { "Entity " + std::to_string(i), i , i * 5.5f } );
                    nbB++;
                }
                    
                if(i % 3 == 0)
                {
                    ecs.attach<Pattern>(entity, { "Guess i got pattern " + std::to_string(i % 2) } );
                    nbPattern++;
                }

                if(i % 6 == 0)
                    nbGroupB++;
                    
                ecs.attach<Vector>(entity, { i, -i, 0 } );
                nbVector++;

                entityList.push_back(entity);

                if(i % 1000 == 999)
                    std::cout << "Created " << i << " Entities !" << std::endl;
            }

            auto group = ecs.group<A, B, Vector>();
            
            for(auto& el : group)
            {
                el.get<B>()->a = 5;
                el.get<A>()->a = 12;
                el.get<Vector>()->z += 10;
                el.get<A>()->name = "Hut";
            }

            for(auto& element : *groupA)
            {
                element.get<A>()->a++;
                element.get<Vector>()->z += 0.25f;
            }

            for(auto j = 0; j < n; j++)
            {
                auto i = indices[j];
                ecs.removeEntity(entityList[i]);

                if(debug)
                    print3DContainer(ecs.view<Vector>(), "Printing every vector");

                nbA--;

                if(i % 2 == 0)
                    nbB--;
                    
                if(i % 3 == 0)
                    nbPattern--;

                if(i % 6 == 0)
                    nbGroupB--;

                nbVector--;

                //std::cout << nbA << " : " << viewLen(ecs.view<A>()) << std::endl;
                if(nbA != viewLen(ecs.view<A>()))
                {
                    std::cout << "View A mismatched" << std::endl;
                    throw(-1);
                }

                if(nbB != viewLen(ecs.view<B>()))
                {
                    std::cout << "View B mismatched" << std::endl;
                    throw(-1);
                }

                if(nbPattern != viewLen(ecs.view<Pattern>()))
                {
                    std::cout << "View Pattern mismatched" << std::endl;
                    throw(-1);
                }

                if(nbVector != viewLen(ecs.view<Vector>()))
                {
                    std::cout << "View Vector mismatched" << std::endl;
                    throw(-1);
                }

                if(groupA->size() != nbA)
                {
                    std::cout << "Group A failure" << std::endl;
                    throw(-1);
                }

                if(groupB->size() != nbGroupB)
                {
                    std::cout << "Group B failure" << std::endl;
                    throw(-1);
                }

                if(groupC->size() != nbGroupB)
                {
                    std::cout << "Group C failure" << std::endl;
                    throw(-1);
                }

                if(groupD->size() != nbPattern)
                {
                    std::cout << "Group D failure" << std::endl;
                    throw(-1);
                }

                if(groupE->size() != nbVector)
                {
                    std::cout << "Group E failure" << std::endl;
                    throw(-1);
                }

                for(auto& element : *groupA)
                {
                    element.get<A>()->a++;
                    element.get<Vector>()->z += 0.25f;
                }

                for(auto& element : *groupB)
                {
                    element.get<A>()->a++;
                }

                for(auto& element : *groupC)
                {
                    element.get<B>()->b += 0.1f;
                }

                for(auto& element : *groupD)
                {
                    element.get<Pattern>()->patternName = "Hello " + std::to_string(element.get<Vector>()->x);
                }

                for(auto& element : *groupE)
                {
                    element.get<Vector>()->x += 0.5f;
                }

                if(j % 1000 == 999)
                    std::cout << "Deleted " << j << " Entities !" << std::endl;
                
            }

            print3DContainer(ecs.view<Vector>(), "Printing every vector");
        }

        }
*/
    }
}