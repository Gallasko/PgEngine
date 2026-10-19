#include "stdafx.h"

#include <gtest/gtest.h>

#include "ECS/entitysystem.h"
#include "Scene/scenemanager.h"

#include "mocklogger.h"

namespace pg
{
    namespace test
    {
        namespace
        {
            // What the probe's and the scene's destructors saw, read once the EntitySystem is gone
            struct Teardown
            {
                bool probeDeleted = false;      // ~ProbeSystem has run
                bool sceneDeleted = false;      // ~ProbeScene has run
                bool probeFound = false;        // The scene's getSystem<ProbeSystem>() gave a system
                bool probeAliveForScene = false; // And it was not deleted yet when the scene went
                int probeValue = 0;             // What the scene read from it then
                int scenesDeleted = 0;
            };

            Teardown teardown;

            // Created before the scene system: a lower id, so ~EntitySystem deletes it first
            struct ProbeSystem : public System<>
            {
                ~ProbeSystem() override { value = 0; teardown.probeDeleted = true; }

                int value = 42;
            };

            // A scene that reaches another system as it goes, as a scene's members may
            struct ProbeScene : public Scene
            {
                void init() override {}

                ~ProbeScene() override
                {
                    teardown.sceneDeleted = true;
                    teardown.scenesDeleted++;

                    auto probe = getSystem<ProbeSystem>();

                    teardown.probeFound = probe != nullptr;
                    teardown.probeAliveForScene = not teardown.probeDeleted;

                    // Read only from a system still there: a deleted one would be a use after free
                    if (probe and teardown.probeAliveForScene)
                        teardown.probeValue = probe->value;
                }
            };
        }

        // The scene loaded last is deleted with its EntitySystem, while the systems it may reach still stand
        TEST(scenesystem_test, the_scene_is_freed_with_the_ecs_before_any_system)
        {
            MockLogger logger;
            teardown = Teardown{};

            {
                EntitySystem ecs;

                auto probe = ecs.createSystem<ProbeSystem>();
                auto scenes = ecs.createSystem<SceneElementSystem>();

                // The ordering this test pins needs the probe deleted before the scene system
                ASSERT_LT(probe->_id, scenes->_id);

                scenes->loadSystemScene<ProbeScene>();

                for (int i = 0; i < 12; ++i)
                    ecs.executeOnce();

                ASSERT_NE(dynamic_cast<ProbeScene*>(scenes->systemScene), nullptr);
                EXPECT_FALSE(teardown.sceneDeleted);
            }

            EXPECT_TRUE(teardown.sceneDeleted);
            EXPECT_EQ(teardown.scenesDeleted, 1);
            EXPECT_TRUE(teardown.probeFound);
            EXPECT_TRUE(teardown.probeAliveForScene);
            EXPECT_EQ(teardown.probeValue, 42);
            EXPECT_TRUE(teardown.probeDeleted);
        }

        // A scene asked for and not loaded yet is freed too, once
        TEST(scenesystem_test, a_scene_waiting_to_load_is_freed_with_the_ecs)
        {
            MockLogger logger;
            teardown = Teardown{};

            {
                EntitySystem ecs;

                ecs.createSystem<ProbeSystem>();
                auto scenes = ecs.createSystem<SceneElementSystem>();

                scenes->loadSystemScene<ProbeScene>();

                ASSERT_NE(scenes->nextSystemScene, nullptr);
                EXPECT_EQ(scenes->systemScene, nullptr);
            }

            EXPECT_EQ(teardown.scenesDeleted, 1);
            EXPECT_TRUE(teardown.probeAliveForScene);
        }

        // A scene replaced by another is deleted then, and the new one with the ecs: each once
        TEST(scenesystem_test, a_replaced_scene_and_the_last_are_each_freed_once)
        {
            MockLogger logger;
            teardown = Teardown{};

            {
                EntitySystem ecs;

                ecs.createSystem<ProbeSystem>();
                auto scenes = ecs.createSystem<SceneElementSystem>();

                scenes->loadSystemScene<ProbeScene>();

                for (int i = 0; i < 12; ++i)
                    ecs.executeOnce();

                scenes->loadSystemScene<ProbeScene>();

                for (int i = 0; i < 12; ++i)
                    ecs.executeOnce();

                EXPECT_EQ(teardown.scenesDeleted, 1);
            }

            EXPECT_EQ(teardown.scenesDeleted, 2);
            EXPECT_TRUE(teardown.probeAliveForScene);
        }
    }
}
