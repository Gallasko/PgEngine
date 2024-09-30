// #include "gtest/gtest.h"

// #include "2D/collisionsystem.h"

// #include "mocklogger.h"

// namespace pg
// {
//     namespace test
//     {
//         // ----------------------------------------------------------------------------------------
//         // ---------------------------        Test separator        -------------------------------
//         // ----------------------------------------------------------------------------------------
//         TEST(system_collision_2d, initialization)
//         {
//             EntitySystem ecs;

//             ecs.createSystem<UiComponentSystem>();

//             auto sys = ecs.createSystem<CollisionSystem>();

//             EXPECT_EQ(sys->pageSize.x, 5.0f);
//             EXPECT_EQ(sys->pageSize.y, 5.0f);

//             EXPECT_EQ(sys->cellSize.x, 5.0f);
//             EXPECT_EQ(sys->cellSize.y, 5.0f);

//             EXPECT_EQ(sys->loadedPages.size(), 0);
//         }

//         TEST(system_collision_2d, creation)
//         {
//             EntitySystem ecs;

//             ecs.createSystem<UiComponentSystem>();

//             auto sys = ecs.createSystem<CollisionSystem>();

//             auto ent = ecs.createEntity();

//             auto entbis = ecs.createEntity();

//             EXPECT_EQ(sys->pageSize.x, 5.0f);
//             EXPECT_EQ(sys->pageSize.y, 5.0f);

//             EXPECT_EQ(sys->cellSize.x, 5.0f);
//             EXPECT_EQ(sys->cellSize.y, 5.0f);

//             EXPECT_EQ(sys->loadedPages.size(), 0);

//             auto collCompBis = ecs.attach<CollisionComponent>(entbis);

//             EXPECT_EQ(collCompBis->cells.size(), 0);

//             ecs.attach<UiComponent>(entbis);

//             EXPECT_EQ(collCompBis->cells.size(), 0);
//             EXPECT_EQ(sys->loadedPages.size(), 0);

//             auto uiComp = ecs.attach<UiComponent>(ent);

//             uiComp->setWidth(1.0f);
//             uiComp->setHeight(1.0f);

//             auto collComp = ecs.attach<CollisionComponent>(ent);

//             EXPECT_EQ(collComp->cells.size(), 1);
            
//             if (collComp->cells.size() > 0)
//             {
//                 EXPECT_EQ(collComp->cells[0]->pagePos.x, 0);
//                 EXPECT_EQ(collComp->cells[0]->pagePos.y, 0);

//                 EXPECT_EQ(collComp->cells[0]->pos.x, 0);
//                 EXPECT_EQ(collComp->cells[0]->pos.y, 0);
//             }
            
//             EXPECT_EQ(sys->loadedPages.size(), 1);
//         }

//         TEST(system_collision_2d, page_overlap_but_size_less_than_page_size)
//         {
//             EntitySystem ecs;

//             ecs.createSystem<UiComponentSystem>();

//             auto sys = ecs.createSystem<CollisionSystem>();

//             auto ent = ecs.createEntity();
//             auto ent1 = ecs.createEntity();

//             auto uiComp = ecs.attach<UiComponent>(ent);

//             uiComp->setX(8.0f);

//             uiComp->setWidth(20.0f);
//             uiComp->setHeight(1.0f);

//             auto collComp = ecs.attach<CollisionComponent>(ent);

//             EXPECT_EQ(collComp->cells.size(), 5);
            
//             EXPECT_EQ(sys->loadedPages.size(), 2);

//             auto uiComp1 = ecs.attach<UiComponent>(ent1);

//             uiComp1->setX(8.0f);

//             uiComp1->setWidth(45.0f);
//             uiComp1->setHeight(1.0f);

//             auto collComp1 = ecs.attach<CollisionComponent>(ent1);

//             EXPECT_EQ(collComp1->cells.size(), 10);
            
//             EXPECT_EQ(sys->loadedPages.size(), 3);
//         }
//     }
// }