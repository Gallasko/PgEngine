#include "gtest/gtest.h"

#include "2D/simple2dobject.h"

namespace pg
{
    namespace test
    {
        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(system_2d, initialization)
        {
            MasterRenderer renderer;

            Simple2DObjectSystem sys(&renderer);

            EXPECT_EQ(sys.currentSize,     0);
            EXPECT_EQ(sys.elementIndex,    0);
            EXPECT_EQ(sys.visibleElements, 0);
        }

        TEST(system_2d, creation)
        {
            EntitySystem ecs;

            MasterRenderer renderer;

            ecs.createSystem<UiComponentSystem>();

            auto sys = ecs.createSystem<Simple2DObjectSystem>(&renderer);

            EXPECT_EQ(sys->currentSize,     1);
            EXPECT_EQ(sys->elementIndex,    0);
            EXPECT_EQ(sys->visibleElements, 0);

            auto ent = ecs.createEntity();

            EXPECT_EQ(sys->currentSize,     1);
            EXPECT_EQ(sys->elementIndex,    0);
            EXPECT_EQ(sys->visibleElements, 0);

            auto ui = ecs.attach<UiComponent>(ent);

            ui->setX(15);
            ui->setY(16);

            ui->setWidth(30);
            ui->setHeight(40);
            
            ecs.attach<Simple2DObject>(ent, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     2);
            EXPECT_EQ(sys->elementIndex,    1);
            EXPECT_EQ(sys->visibleElements, 1);

            EXPECT_EQ(sys->bufferData[0], 15.0f);
            EXPECT_EQ(sys->bufferData[1], 16.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 30.0f);
            EXPECT_EQ(sys->bufferData[4], 40.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);
        }

        TEST(system_2d, multiple_creation)
        {
            EntitySystem ecs;

            MasterRenderer renderer;

            ecs.createSystem<UiComponentSystem>();

            auto sys = ecs.createSystem<Simple2DObjectSystem>(&renderer);

            EXPECT_EQ(sys->currentSize,     1);
            EXPECT_EQ(sys->elementIndex,    0);
            EXPECT_EQ(sys->visibleElements, 0);

            auto ent0 = ecs.createEntity();
            auto ent1 = ecs.createEntity();
            auto ent2 = ecs.createEntity();
            auto ent3 = ecs.createEntity();
            auto ent4 = ecs.createEntity();

            ecs.attach<UiComponent>(ent0);
            ecs.attach<Simple2DObject>(ent0, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     2);
            EXPECT_EQ(sys->elementIndex,    1);
            EXPECT_EQ(sys->visibleElements, 1);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);

            ecs.attach<UiComponent>(ent1);
            ecs.attach<Simple2DObject>(ent1, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    2);
            EXPECT_EQ(sys->visibleElements, 2);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);

            ecs.attach<UiComponent>(ent2);
            ecs.attach<Simple2DObject>(ent2, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    3);
            EXPECT_EQ(sys->visibleElements, 3);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);

            auto ui = ecs.attach<UiComponent>(ent3);
            ui->setVisibility(false);
            ecs.attach<Simple2DObject>(ent3, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    4);
            EXPECT_EQ(sys->visibleElements, 3);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 3);

            ecs.attach<UiComponent>(ent4);
            ecs.attach<Simple2DObject>(ent4, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    5);
            EXPECT_EQ(sys->visibleElements, 4);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 4);
            EXPECT_EQ(sys->idToIndexMap[ent4.id], 3);
        }

        TEST(system_2d, make_2D_helper)
        {
            EntitySystem ecs;

            MasterRenderer renderer;

            ecs.createSystem<UiComponentSystem>();

            auto sys = ecs.createSystem<Simple2DObjectSystem>(&renderer);

            EXPECT_EQ(sys->currentSize,     1);
            EXPECT_EQ(sys->elementIndex,    0);
            EXPECT_EQ(sys->visibleElements, 0);

            makeSimple2DShape(&ecs, Shape2D::Square, 10, 20, {255.0f, 255.0f, 255.0f});

            EXPECT_EQ(sys->currentSize,     2);
            EXPECT_EQ(sys->elementIndex,    1);
            EXPECT_EQ(sys->visibleElements, 1);

            makeSimple2DShape(&ecs, Shape2D::Square, 10, 20, {255.0f, 255.0f, 255.0f});

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    2);
            EXPECT_EQ(sys->visibleElements, 2);
        }

        TEST(system_2d, visibility_change)
        {
            EntitySystem ecs;

            MasterRenderer renderer;

            ecs.createSystem<UiComponentSystem>();

            auto sys = ecs.createSystem<Simple2DObjectSystem>(&renderer);

            EXPECT_EQ(sys->currentSize,     1);
            EXPECT_EQ(sys->elementIndex,    0);
            EXPECT_EQ(sys->visibleElements, 0);

            auto ent0 = ecs.createEntity();
            auto ent1 = ecs.createEntity();
            auto ent2 = ecs.createEntity();
            auto ent3 = ecs.createEntity();
            auto ent4 = ecs.createEntity();

            ecs.attach<UiComponent>(ent0);
            ecs.attach<Simple2DObject>(ent0, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     2);
            EXPECT_EQ(sys->elementIndex,    1);
            EXPECT_EQ(sys->visibleElements, 1);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);

            ecs.attach<UiComponent>(ent1);
            ecs.attach<Simple2DObject>(ent1, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    2);
            EXPECT_EQ(sys->visibleElements, 2);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);

            ecs.attach<UiComponent>(ent2);
            ecs.attach<Simple2DObject>(ent2, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    3);
            EXPECT_EQ(sys->visibleElements, 3);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);

            auto ui = ecs.attach<UiComponent>(ent3);
            ui->setVisibility(false);
            ecs.attach<Simple2DObject>(ent3, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    4);
            EXPECT_EQ(sys->visibleElements, 3);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 3);

            ecs.attach<UiComponent>(ent4);
            ecs.attach<Simple2DObject>(ent4, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    5);
            EXPECT_EQ(sys->visibleElements, 4);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 4);
            EXPECT_EQ(sys->idToIndexMap[ent4.id], 3);

            ui->setVisibility(true);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    5);
            EXPECT_EQ(sys->visibleElements, 5);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);

            // Should stay "swapped" because the last index is the last visible element !
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 4);
            EXPECT_EQ(sys->idToIndexMap[ent4.id], 3);
        }

        TEST(system_2d, multiple_visibility_change)
        {
            EntitySystem ecs;

            MasterRenderer renderer;

            ecs.createSystem<UiComponentSystem>();

            auto sys = ecs.createSystem<Simple2DObjectSystem>(&renderer);

            EXPECT_EQ(sys->currentSize,     1);
            EXPECT_EQ(sys->elementIndex,    0);
            EXPECT_EQ(sys->visibleElements, 0);

            auto ent0 = ecs.createEntity();
            auto ent1 = ecs.createEntity();
            auto ent2 = ecs.createEntity();
            auto ent3 = ecs.createEntity();
            auto ent4 = ecs.createEntity();

            auto ui0 = ecs.attach<UiComponent>(ent0);
            ecs.attach<Simple2DObject>(ent0, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     2);
            EXPECT_EQ(sys->elementIndex,    1);
            EXPECT_EQ(sys->visibleElements, 1);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);

            ecs.attach<UiComponent>(ent1);
            ecs.attach<Simple2DObject>(ent1, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    2);
            EXPECT_EQ(sys->visibleElements, 2);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);

            ecs.attach<UiComponent>(ent2);
            ecs.attach<Simple2DObject>(ent2, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    3);
            EXPECT_EQ(sys->visibleElements, 3);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);

            auto ui = ecs.attach<UiComponent>(ent3);
            ui->setVisibility(false);
            ecs.attach<Simple2DObject>(ent3, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    4);
            EXPECT_EQ(sys->visibleElements, 3);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 3);

            ecs.attach<UiComponent>(ent4);
            ecs.attach<Simple2DObject>(ent4, Shape2D::Square);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    5);
            EXPECT_EQ(sys->visibleElements, 4);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 0);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 4);
            EXPECT_EQ(sys->idToIndexMap[ent4.id], 3);

            ui0->setVisibility(false);
            
            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    5);
            EXPECT_EQ(sys->visibleElements, 3);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 3);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 4);
            EXPECT_EQ(sys->idToIndexMap[ent4.id], 0);

            ui->setVisibility(true);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    5);
            EXPECT_EQ(sys->visibleElements, 4);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 4);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 3);
            EXPECT_EQ(sys->idToIndexMap[ent4.id], 0);

            ui0->setVisibility(true);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    5);
            EXPECT_EQ(sys->visibleElements, 5);

            EXPECT_EQ(sys->idToIndexMap[ent0.id], 4);
            EXPECT_EQ(sys->idToIndexMap[ent1.id], 1);
            EXPECT_EQ(sys->idToIndexMap[ent2.id], 2);
            EXPECT_EQ(sys->idToIndexMap[ent3.id], 3);
            EXPECT_EQ(sys->idToIndexMap[ent4.id], 0);
        }

        TEST(system_2d, buffer_values)
        {
            EntitySystem ecs;

            MasterRenderer renderer;

            ecs.createSystem<UiComponentSystem>();

            auto sys = ecs.createSystem<Simple2DObjectSystem>(&renderer);

            EXPECT_EQ(sys->currentSize,     1);
            EXPECT_EQ(sys->elementIndex,    0);
            EXPECT_EQ(sys->visibleElements, 0);

            makeSimple2DShape(&ecs, Shape2D::Square, 10, 20, {255.0f, 255.0f, 255.0f});

            EXPECT_EQ(sys->currentSize,     2);
            EXPECT_EQ(sys->elementIndex,    1);
            EXPECT_EQ(sys->visibleElements, 1);

            EXPECT_EQ(sys->bufferData[0], 0.0f);
            EXPECT_EQ(sys->bufferData[1], 0.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 10.0f);
            EXPECT_EQ(sys->bufferData[4], 20.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);

            makeSimple2DShape(&ecs, Shape2D::Square, 40, 50, {255.0f, 255.0f, 255.0f});

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    2);
            EXPECT_EQ(sys->visibleElements, 2);

            EXPECT_EQ(sys->bufferData[0], 0.0f);
            EXPECT_EQ(sys->bufferData[1], 0.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 10.0f);
            EXPECT_EQ(sys->bufferData[4], 20.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);

            EXPECT_EQ(sys->bufferData[8], 0.0f);
            EXPECT_EQ(sys->bufferData[9], 0.0f);
            EXPECT_EQ(sys->bufferData[10], 0.0f);

            EXPECT_EQ(sys->bufferData[11], 40.0f);
            EXPECT_EQ(sys->bufferData[12], 50.0f);

            EXPECT_EQ(sys->bufferData[13], 255.0f);
            EXPECT_EQ(sys->bufferData[14], 255.0f);
            EXPECT_EQ(sys->bufferData[15], 255.0f);

            auto list2 = makeSimple2DShape(&ecs, Shape2D::Square, 60, 70, {255.0f, 255.0f, 255.0f});
            list2.get<UiComponent>()->setVisibility(false);

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    3);
            EXPECT_EQ(sys->visibleElements, 2);

            EXPECT_EQ(sys->bufferData[0], 0.0f);
            EXPECT_EQ(sys->bufferData[1], 0.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 10.0f);
            EXPECT_EQ(sys->bufferData[4], 20.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);

            EXPECT_EQ(sys->bufferData[8], 0.0f);
            EXPECT_EQ(sys->bufferData[9], 0.0f);
            EXPECT_EQ(sys->bufferData[10], 0.0f);

            EXPECT_EQ(sys->bufferData[11], 40.0f);
            EXPECT_EQ(sys->bufferData[12], 50.0f);

            EXPECT_EQ(sys->bufferData[13], 255.0f);
            EXPECT_EQ(sys->bufferData[14], 255.0f);
            EXPECT_EQ(sys->bufferData[15], 255.0f);

            EXPECT_EQ(sys->bufferData[16], 0.0f);
            EXPECT_EQ(sys->bufferData[17], 0.0f);
            EXPECT_EQ(sys->bufferData[18], 0.0f);

            EXPECT_EQ(sys->bufferData[19], 60.0f);
            EXPECT_EQ(sys->bufferData[20], 70.0f);

            EXPECT_EQ(sys->bufferData[21], 255.0f);
            EXPECT_EQ(sys->bufferData[22], 255.0f);
            EXPECT_EQ(sys->bufferData[23], 255.0f);

            makeSimple2DShape(&ecs, Shape2D::Square, 80, 90, {255.0f, 255.0f, 255.0f});

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    4);
            EXPECT_EQ(sys->visibleElements, 3);

            EXPECT_EQ(sys->bufferData[0], 0.0f);
            EXPECT_EQ(sys->bufferData[1], 0.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 10.0f);
            EXPECT_EQ(sys->bufferData[4], 20.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);

            EXPECT_EQ(sys->bufferData[8], 0.0f);
            EXPECT_EQ(sys->bufferData[9], 0.0f);
            EXPECT_EQ(sys->bufferData[10], 0.0f);

            EXPECT_EQ(sys->bufferData[11], 40.0f);
            EXPECT_EQ(sys->bufferData[12], 50.0f);

            EXPECT_EQ(sys->bufferData[13], 255.0f);
            EXPECT_EQ(sys->bufferData[14], 255.0f);
            EXPECT_EQ(sys->bufferData[15], 255.0f);

            EXPECT_EQ(sys->bufferData[16], 0.0f);
            EXPECT_EQ(sys->bufferData[17], 0.0f);
            EXPECT_EQ(sys->bufferData[18], 0.0f);

            EXPECT_EQ(sys->bufferData[19], 80.0f);
            EXPECT_EQ(sys->bufferData[20], 90.0f);

            EXPECT_EQ(sys->bufferData[21], 255.0f);
            EXPECT_EQ(sys->bufferData[22], 255.0f);
            EXPECT_EQ(sys->bufferData[23], 255.0f);

            EXPECT_EQ(sys->bufferData[24], 0.0f);
            EXPECT_EQ(sys->bufferData[25], 0.0f);
            EXPECT_EQ(sys->bufferData[26], 0.0f);

            EXPECT_EQ(sys->bufferData[27], 60.0f);
            EXPECT_EQ(sys->bufferData[28], 70.0f);

            EXPECT_EQ(sys->bufferData[29], 255.0f);
            EXPECT_EQ(sys->bufferData[30], 255.0f);
            EXPECT_EQ(sys->bufferData[31], 255.0f);
        }

        TEST(system_2d, buffer_values_with_dettach)
        {
            EntitySystem ecs;

            MasterRenderer renderer;

            ecs.createSystem<UiComponentSystem>();

            auto sys = ecs.createSystem<Simple2DObjectSystem>(&renderer);

            EXPECT_EQ(sys->currentSize,     1);
            EXPECT_EQ(sys->elementIndex,    0);
            EXPECT_EQ(sys->visibleElements, 0);

            auto list0 = makeSimple2DShape(&ecs, Shape2D::Square, 10, 20, {255.0f, 255.0f, 255.0f});

            EXPECT_EQ(sys->currentSize,     2);
            EXPECT_EQ(sys->elementIndex,    1);
            EXPECT_EQ(sys->visibleElements, 1);

            EXPECT_EQ(sys->bufferData[0], 0.0f);
            EXPECT_EQ(sys->bufferData[1], 0.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 10.0f);
            EXPECT_EQ(sys->bufferData[4], 20.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);

            makeSimple2DShape(&ecs, Shape2D::Square, 40, 50, {255.0f, 255.0f, 255.0f});

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    2);
            EXPECT_EQ(sys->visibleElements, 2);

            EXPECT_EQ(sys->bufferData[0], 0.0f);
            EXPECT_EQ(sys->bufferData[1], 0.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 10.0f);
            EXPECT_EQ(sys->bufferData[4], 20.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);

            EXPECT_EQ(sys->bufferData[8], 0.0f);
            EXPECT_EQ(sys->bufferData[9], 0.0f);
            EXPECT_EQ(sys->bufferData[10], 0.0f);

            EXPECT_EQ(sys->bufferData[11], 40.0f);
            EXPECT_EQ(sys->bufferData[12], 50.0f);

            EXPECT_EQ(sys->bufferData[13], 255.0f);
            EXPECT_EQ(sys->bufferData[14], 255.0f);
            EXPECT_EQ(sys->bufferData[15], 255.0f);

            auto list2 = makeSimple2DShape(&ecs, Shape2D::Square, 60, 70, {255.0f, 255.0f, 255.0f});
            list2.get<UiComponent>()->setVisibility(false);

            EXPECT_EQ(sys->currentSize,     4);
            EXPECT_EQ(sys->elementIndex,    3);
            EXPECT_EQ(sys->visibleElements, 2);

            EXPECT_EQ(sys->bufferData[0], 0.0f);
            EXPECT_EQ(sys->bufferData[1], 0.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 10.0f);
            EXPECT_EQ(sys->bufferData[4], 20.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);

            EXPECT_EQ(sys->bufferData[8], 0.0f);
            EXPECT_EQ(sys->bufferData[9], 0.0f);
            EXPECT_EQ(sys->bufferData[10], 0.0f);

            EXPECT_EQ(sys->bufferData[11], 40.0f);
            EXPECT_EQ(sys->bufferData[12], 50.0f);

            EXPECT_EQ(sys->bufferData[13], 255.0f);
            EXPECT_EQ(sys->bufferData[14], 255.0f);
            EXPECT_EQ(sys->bufferData[15], 255.0f);

            EXPECT_EQ(sys->bufferData[16], 0.0f);
            EXPECT_EQ(sys->bufferData[17], 0.0f);
            EXPECT_EQ(sys->bufferData[18], 0.0f);

            EXPECT_EQ(sys->bufferData[19], 60.0f);
            EXPECT_EQ(sys->bufferData[20], 70.0f);

            EXPECT_EQ(sys->bufferData[21], 255.0f);
            EXPECT_EQ(sys->bufferData[22], 255.0f);
            EXPECT_EQ(sys->bufferData[23], 255.0f);

            makeSimple2DShape(&ecs, Shape2D::Square, 80, 90, {255.0f, 255.0f, 255.0f});

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    4);
            EXPECT_EQ(sys->visibleElements, 3);

            EXPECT_EQ(sys->bufferData[0], 0.0f);
            EXPECT_EQ(sys->bufferData[1], 0.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 10.0f);
            EXPECT_EQ(sys->bufferData[4], 20.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);

            EXPECT_EQ(sys->bufferData[8], 0.0f);
            EXPECT_EQ(sys->bufferData[9], 0.0f);
            EXPECT_EQ(sys->bufferData[10], 0.0f);

            EXPECT_EQ(sys->bufferData[11], 40.0f);
            EXPECT_EQ(sys->bufferData[12], 50.0f);

            EXPECT_EQ(sys->bufferData[13], 255.0f);
            EXPECT_EQ(sys->bufferData[14], 255.0f);
            EXPECT_EQ(sys->bufferData[15], 255.0f);

            EXPECT_EQ(sys->bufferData[16], 0.0f);
            EXPECT_EQ(sys->bufferData[17], 0.0f);
            EXPECT_EQ(sys->bufferData[18], 0.0f);

            EXPECT_EQ(sys->bufferData[19], 80.0f);
            EXPECT_EQ(sys->bufferData[20], 90.0f);

            EXPECT_EQ(sys->bufferData[21], 255.0f);
            EXPECT_EQ(sys->bufferData[22], 255.0f);
            EXPECT_EQ(sys->bufferData[23], 255.0f);

            EXPECT_EQ(sys->bufferData[24], 0.0f);
            EXPECT_EQ(sys->bufferData[25], 0.0f);
            EXPECT_EQ(sys->bufferData[26], 0.0f);

            EXPECT_EQ(sys->bufferData[27], 60.0f);
            EXPECT_EQ(sys->bufferData[28], 70.0f);

            EXPECT_EQ(sys->bufferData[29], 255.0f);
            EXPECT_EQ(sys->bufferData[30], 255.0f);
            EXPECT_EQ(sys->bufferData[31], 255.0f);

            ecs.dettach<UiComponent>(list0.entity);

            EXPECT_EQ(sys->currentSize,     8);
            EXPECT_EQ(sys->elementIndex,    3);
            EXPECT_EQ(sys->visibleElements, 2);

            EXPECT_EQ(sys->bufferData[0], 0.0f);
            EXPECT_EQ(sys->bufferData[1], 0.0f);
            EXPECT_EQ(sys->bufferData[2], 0.0f);

            EXPECT_EQ(sys->bufferData[3], 80.0f);
            EXPECT_EQ(sys->bufferData[4], 90.0f);

            EXPECT_EQ(sys->bufferData[5], 255.0f);
            EXPECT_EQ(sys->bufferData[6], 255.0f);
            EXPECT_EQ(sys->bufferData[7], 255.0f);

            EXPECT_EQ(sys->bufferData[8], 0.0f);
            EXPECT_EQ(sys->bufferData[9], 0.0f);
            EXPECT_EQ(sys->bufferData[10], 0.0f);

            EXPECT_EQ(sys->bufferData[11], 40.0f);
            EXPECT_EQ(sys->bufferData[12], 50.0f);

            EXPECT_EQ(sys->bufferData[13], 255.0f);
            EXPECT_EQ(sys->bufferData[14], 255.0f);
            EXPECT_EQ(sys->bufferData[15], 255.0f);

            EXPECT_EQ(sys->bufferData[16], 0.0f);
            EXPECT_EQ(sys->bufferData[17], 0.0f);
            EXPECT_EQ(sys->bufferData[18], 0.0f);

            EXPECT_EQ(sys->bufferData[19], 60.0f);
            EXPECT_EQ(sys->bufferData[20], 70.0f);

            EXPECT_EQ(sys->bufferData[21], 255.0f);
            EXPECT_EQ(sys->bufferData[22], 255.0f);
            EXPECT_EQ(sys->bufferData[23], 255.0f);
        }

    }
}