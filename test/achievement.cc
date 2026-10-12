#include "stdafx.h"

#include <gtest/gtest.h>

#include "Systems/achievement.h"
#include "Systems/gamefacts.h"

#include "ECS/entitysystem.h"

#include "mocklogger.h"
#include "factfeed.h"

namespace pg
{
    namespace test
    {
        namespace
        {
            // Records the name carried by every unlock event
            struct UnlockRecorder : public System<Listener<StandardEvent>, InitSys, StoragePolicy>
            {
                virtual std::string getSystemName() const override { return "Unlock Recorder"; }

                virtual void init() override
                {
                    addListenerToStandardEvent(AchievementUnlockEventName);
                }

                virtual void onEvent(const StandardEvent& event) override
                {
                    if (event.name != AchievementUnlockEventName)
                        return;

                    unlocked.push_back(event.values.at("name").toString());
                }

                std::vector<std::string> unlocked;
            };

            // Like the facts, the system would load from and save to save/systems.sz
            AchievementSys* createTestAchievements(EntitySystem *ecs)
            {
                auto achievements = ecs->createSystem<AchievementSys>();

                ecs->getComponentRegistry()->unregisterSystemSave(achievements->getSystemName());

                achievements->clear();

                return achievements;
            }

            Achievement makeAchievement(const std::string& name, const std::vector<FactChecker>& asks)
            {
                Achievement achievement;

                achievement.name = name;
                achievement.prerequisiteFacts = asks;

                return achievement;
            }

            void frames(EntitySystem& ecs, int nbFrames)
            {
                for (int i = 0; i < nbFrames; ++i)
                    ecs.executeOnce();
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(achievement_test, unlock_on_fact_change)
        {
            MockLogger logger;
            EntitySystem ecs;

            auto facts = createTestFacts(&ecs);
            auto achievements = createTestAchievements(&ecs);
            auto recorder = ecs.createSystem<UnlockRecorder>();

            achievements->addNewAchivement(makeAchievement("rich", {FactChecker("gold", 10, FactCheckEquality::GreaterEqual)}));

            frames(ecs, 3);

            EXPECT_EQ(achievements->achievementLocked.size(), 1u);
            EXPECT_TRUE(achievements->achievementUnlocked.empty());

            ecs.sendEvent(AddFact{"gold", ElementType{4}});
            frames(ecs, 3);

            EXPECT_TRUE(achievements->achievementUnlocked.empty());

            ecs.sendEvent(IncreaseFact{"gold", 6});
            frames(ecs, 3);

            ASSERT_EQ(achievements->achievementUnlocked.size(), 1u);
            EXPECT_EQ(achievements->achievementUnlocked[0].name, "rich");
            EXPECT_TRUE(achievements->achievementUnlocked[0].unlocked);
            EXPECT_TRUE(achievements->achievementLocked.empty());

            EXPECT_TRUE(facts->getFact<bool>("rich_unlocked", false));

            ASSERT_EQ(recorder->unlocked.size(), 1u);
            EXPECT_EQ(recorder->unlocked[0], "rich");

            // Unlocked once: a later change does not send it again
            ecs.sendEvent(IncreaseFact{"gold", 6});
            frames(ecs, 3);

            EXPECT_EQ(recorder->unlocked.size(), 1u);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(achievement_test, every_prerequisite_is_needed)
        {
            MockLogger logger;
            EntitySystem ecs;

            createTestFacts(&ecs);
            auto achievements = createTestAchievements(&ecs);

            achievements->addNewAchivement(makeAchievement("knight", {
                FactChecker("strength", 18, FactCheckEquality::GreaterEqual),
                FactChecker("sword", 4, FactCheckEquality::GreaterEqual)}));

            frames(ecs, 3);

            ecs.sendEvent(AddFact{"strength", ElementType{18}});
            frames(ecs, 3);

            EXPECT_TRUE(achievements->achievementUnlocked.empty());

            ecs.sendEvent(AddFact{"sword", ElementType{4}});
            frames(ecs, 3);

            ASSERT_EQ(achievements->achievementUnlocked.size(), 1u);
            EXPECT_EQ(achievements->achievementUnlocked[0].name, "knight");
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A prerequisite that held and no longer does is still waited on: the achievement is the
        // moment they all hold together, whichever order they came to hold in.
        TEST(achievement_test, a_fact_that_stops_holding_is_waited_on_again)
        {
            MockLogger logger;
            EntitySystem ecs;

            createTestFacts(&ecs);
            auto achievements = createTestAchievements(&ecs);
            auto recorder = ecs.createSystem<UnlockRecorder>();

            // Hungry, and of age: his store runs low as a boy, fills, and runs low again as a man
            achievements->addNewAchivement(makeAchievement("hungry", {
                FactChecker("rations", 3, FactCheckEquality::LesserEqual),
                FactChecker("age", 7, FactCheckEquality::GreaterEqual)}));

            ecs.sendEvent(AddFact{"rations", ElementType{10}});
            ecs.sendEvent(AddFact{"age", ElementType{6}});
            frames(ecs, 3);

            ecs.sendEvent(AddFact{"rations", ElementType{2}});
            frames(ecs, 3);

            EXPECT_TRUE(achievements->achievementUnlocked.empty());

            ecs.sendEvent(AddFact{"rations", ElementType{12}});
            frames(ecs, 3);

            // Of age, with a full store: not yet
            ecs.sendEvent(AddFact{"age", ElementType{7}});
            frames(ecs, 3);

            EXPECT_TRUE(achievements->achievementUnlocked.empty());
            EXPECT_EQ(achievements->achievementLocked.size(), 1u);

            ecs.sendEvent(AddFact{"rations", ElementType{3}});
            frames(ecs, 3);

            ASSERT_EQ(achievements->achievementUnlocked.size(), 1u);
            EXPECT_EQ(achievements->achievementUnlocked[0].name, "hungry");
            EXPECT_TRUE(achievements->achievementLocked.empty());

            // Once: both of its facts move again, and nothing is sent
            ecs.sendEvent(AddFact{"age", ElementType{8}});
            ecs.sendEvent(AddFact{"rations", ElementType{1}});
            frames(ecs, 3);

            EXPECT_EQ(recorder->unlocked.size(), 1u);
            EXPECT_EQ(achievements->achievementUnlocked.size(), 1u);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // The same for a fact that holds the day the achievement is added and has stopped holding
        // when the other comes: it is waited on like the others.
        TEST(achievement_test, a_fact_that_held_when_added_is_waited_on)
        {
            MockLogger logger;
            EntitySystem ecs;

            createTestFacts(&ecs);
            auto achievements = createTestAchievements(&ecs);

            ecs.sendEvent(AddFact{"rations", ElementType{2}});
            ecs.sendEvent(AddFact{"age", ElementType{6}});
            frames(ecs, 3);

            achievements->addNewAchivement(makeAchievement("hungry", {
                FactChecker("rations", 3, FactCheckEquality::LesserEqual),
                FactChecker("age", 7, FactCheckEquality::GreaterEqual)}));

            frames(ecs, 3);

            ecs.sendEvent(AddFact{"rations", ElementType{12}});
            ecs.sendEvent(AddFact{"age", ElementType{7}});
            frames(ecs, 3);

            EXPECT_TRUE(achievements->achievementUnlocked.empty());

            ecs.sendEvent(AddFact{"rations", ElementType{1}});
            frames(ecs, 3);

            ASSERT_EQ(achievements->achievementUnlocked.size(), 1u);
            EXPECT_EQ(achievements->achievementUnlocked[0].name, "hungry");
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(achievement_test, already_true_when_added)
        {
            MockLogger logger;
            EntitySystem ecs;

            auto facts = createTestFacts(&ecs);
            auto achievements = createTestAchievements(&ecs);
            auto recorder = ecs.createSystem<UnlockRecorder>();

            ecs.sendEvent(AddFact{"gold", ElementType{50}});
            frames(ecs, 3);

            achievements->addNewAchivement(makeAchievement("rich", {FactChecker("gold", 10, FactCheckEquality::GreaterEqual)}));
            frames(ecs, 3);

            ASSERT_EQ(achievements->achievementUnlocked.size(), 1u);
            EXPECT_TRUE(achievements->achievementUnlocked[0].unlocked);
            EXPECT_TRUE(achievements->achievementLocked.empty());

            EXPECT_TRUE(facts->getFact<bool>("rich_unlocked", false));
            EXPECT_EQ(recorder->unlocked.size(), 1u);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(achievement_test, rewards_are_sent)
        {
            MockLogger logger;
            EntitySystem ecs;

            auto facts = createTestFacts(&ecs);
            auto achievements = createTestAchievements(&ecs);

            ecs.sendEvent(AddFact{"gems", ElementType{1}});
            ecs.sendEvent(AddFact{"curse", ElementType{true}});

            auto achievement = makeAchievement("rich", {FactChecker("gold", 10, FactCheckEquality::GreaterEqual)});

            achievement.rewardFacts.push_back(AchievementReward(AddFact{"title", ElementType{std::string("Merchant")}}));
            achievement.rewardFacts.push_back(AchievementReward(IncreaseFact{"gems", 2}));
            achievement.rewardFacts.push_back(AchievementReward(RemoveFact{"curse"}));

            achievements->addNewAchivement(achievement);
            frames(ecs, 3);

            ecs.sendEvent(AddFact{"gold", ElementType{10}});
            frames(ecs, 3);

            EXPECT_EQ(facts->getFact<std::string>("title", ""), "Merchant");
            EXPECT_EQ(facts->getFact<int>("gems"), 3);
            EXPECT_FALSE(facts->hasFact("curse"));
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        TEST(achievement_test, clear_forgets_everything)
        {
            MockLogger logger;
            EntitySystem ecs;

            createTestFacts(&ecs);
            auto achievements = createTestAchievements(&ecs);
            auto recorder = ecs.createSystem<UnlockRecorder>();

            achievements->addNewAchivement(makeAchievement("rich", {FactChecker("gold", 10, FactCheckEquality::GreaterEqual)}));
            achievements->addNewAchivement(makeAchievement("first", {FactChecker("gold", 1, FactCheckEquality::GreaterEqual)}));
            frames(ecs, 3);

            ecs.sendEvent(AddFact{"gold", ElementType{1}});
            frames(ecs, 3);

            EXPECT_EQ(achievements->achievementUnlocked.size(), 1u);
            EXPECT_EQ(achievements->achievementLocked.size(), 1u);

            achievements->clear();

            EXPECT_TRUE(achievements->achievementUnlocked.empty());
            EXPECT_TRUE(achievements->achievementLocked.empty());
            EXPECT_TRUE(achievements->achievementToResolve.empty());

            // Nothing is waited on any more
            ecs.sendEvent(AddFact{"gold", ElementType{100}});
            frames(ecs, 3);

            EXPECT_EQ(recorder->unlocked.size(), 1u);
            EXPECT_TRUE(achievements->achievementUnlocked.empty());
        }
    }
}
