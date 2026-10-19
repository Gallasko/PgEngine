#include "stdafx.h"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/rulevm.h"
#include "Core/walk.h"

#include "ECS/entitysystem.h"

#include "mocklogger.h"

#include "compiledrules.h"

using namespace chronicle;

// A story: a scene played day by day, its tiles offered by the day and by what he chose before,
// as the rules play it (rules/expedition.pg and storylib.pg, asked through Rules::walk the way
// the Life screen asks a walk). The tests' story is `trial` (storytable.pg): four days for four
// months, a lecture for a coin, the library for nothing, an advanced course from the third day
// for who was taught twice, an early tile of the first two days, and a tile taken once.
namespace pg
{
    namespace test
    {
        namespace
        {
            int intOf(const ElementMap& map, const std::string& key)
            {
                auto it = map.find(key);

                if (it == map.end())
                    return 0;

                switch (it->second.type)
                {
                case UnionType::INT:    return it->second.get<int>();
                case UnionType::FLOAT:  return static_cast<int>(it->second.get<float>());
                case UnionType::DOUBLE: return static_cast<int>(it->second.get<double>());
                case UnionType::SIZE_T: return static_cast<int>(it->second.get<size_t>());
                case UnionType::BOOL:   return it->second.get<bool>() ? 1 : 0;
                default:                return 0;
                }
            }

            std::string textOf(const ElementMap& map, const std::string& key)
            {
                auto it = map.find(key);

                return it == map.end() ? std::string() : it->second.toString();
            }

            const ElementMap* byId(const RecordList& list, const std::string& id)
            {
                for (const auto& rec : list)
                {
                    if (textOf(rec, "id") == id)
                        return &rec;
                }

                return nullptr;
            }

            // A youth of sixteen with a little coin put by
            ElementMap student()
            {
                return {{"coin", ElementType{5}}, {"vit", ElementType{40}}, {"vitmax", ElementType{40}}, {"int", ElementType{12}}};
            }

            // One who plays a story: the rules, his character and the scene, each answer taken as the
            // Life screen takes it (the stats become his, the scene becomes the walk as it stands).
            // The Walker of expedition.cc.
            struct Walker
            {
                EntitySystem ecs;
                Rules rules;
                ElementMap character;
                float age = 16.0f;
                WalkState state;
                std::vector<WalkRecipe> recipes;
                WalkQueue queue;
                RecordList walks;
                RuleWalk last;
                int seed = 100;

                explicit Walker(ElementMap who) : character(std::move(who))
                {
                    EXPECT_TRUE(rules.load(&ecs, compiledRules())) << (rules.errors.empty() ? "" : rules.errors.front());

                    rules.lives = 2;
                }

                bool ask(const std::string& what, const WalkAct& act = {}, const std::string& replay = "")
                {
                    const bool ok = rules.walk(age, character, "Aldren", what, ++seed, state, act, recipes, walks, queue, replay, last);

                    EXPECT_TRUE(ok) << (rules.errors.empty() ? "" : rules.errors.front());

                    if (not ok)
                        return false;

                    for (const auto& [key, value] : last.after)
                        character[key] = value;

                    if (what == "enter" or what == "act")
                        state = last.state;

                    return true;
                }

                bool act(const std::string& kind, const std::string& target = "", int count = 0)
                {
                    return ask("act", WalkAct{kind, target, count});
                }

                // A tile taken, as the page sends it: its id is the act's kind
                bool take(const std::string& tile)
                {
                    return act("tile." + tile);
                }

                std::string screen() const { return textOf(last.site, "screen"); }

                const ElementMap* work(const std::string& tile) const { return byId(last.works, "tile." + tile); }

                int day() const { return intOf(state.run, "day"); }
                int mark(const std::string& key) const { return intOf(state.run, "m." + key); }
            };
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // He arrives on the first day: the tiles of that day, and not what asks more of him.
        TEST(story_test, a_story_begins_on_its_first_day)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

            EXPECT_EQ(textOf(w.state.run, "story"), "trial");
            EXPECT_EQ(textOf(w.state.run, "zone"), "");
            EXPECT_EQ(w.day(), 1);
            EXPECT_EQ(intOf(w.state.run, "ap"), 4);
            EXPECT_EQ(w.screen(), "site");

            EXPECT_NE(w.work("lecture"), nullptr);
            EXPECT_NE(w.work("library"), nullptr);
            EXPECT_NE(w.work("early"), nullptr);
            EXPECT_NE(w.work("once"), nullptr);
            EXPECT_EQ(w.work("advanced"), nullptr);

            // A story walks no zone: no beast to seek
            EXPECT_TRUE(w.last.seeks.empty());

            // No story under an unknown name
            Walker lost(student());
            MockLogger quiet;

            EXPECT_FALSE(lost.rules.walk(16.0f, lost.character, "Aldren", "enter", 1, WalkState{}, WalkAct{"story", "nowhere", 0}, {}, {}, WalkQueue{}, "", lost.last));
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A tile is a day: taken, it gives what it gives and the next day comes.
        TEST(story_test, a_tile_taken_ends_the_day)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_TRUE(w.act("tile", "library"));

            EXPECT_EQ(textOf(w.last.site, "refused"), "");
            EXPECT_EQ(w.day(), 2);
            EXPECT_EQ(intOf(w.state.run, "ap"), 3);
            EXPECT_EQ(w.mark("revised"), 1);

            ASSERT_EQ(w.state.record.size(), 1u);
            EXPECT_EQ(textOf(w.state.record.front(), "text"), "Read in the library");

            // The page's own form of it, `tile.<id>` as the act's kind, is the same
            ASSERT_TRUE(w.take("library"));

            EXPECT_EQ(w.day(), 3);
            EXPECT_EQ(w.mark("revised"), 2);
            EXPECT_EQ(w.state.record.size(), 2u);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // What is offered moves with the days and with what he chose: the early tile goes after its
        // second day, the advanced course is shown from the third, shut until he was taught twice.
        TEST(story_test, the_offer_changes_with_the_day)
        {
            MockLogger logger;

            // Two days in the library: not taught, the course is shown and shut
            {
                Walker w(student());

                ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
                ASSERT_TRUE(w.take("library"));

                EXPECT_NE(w.work("early"), nullptr);
                EXPECT_EQ(w.work("advanced"), nullptr);

                ASSERT_TRUE(w.take("library"));

                EXPECT_EQ(w.day(), 3);
                EXPECT_EQ(w.work("early"), nullptr);

                ASSERT_NE(w.work("advanced"), nullptr);
                EXPECT_EQ(intOf(*w.work("advanced"), "locked"), 1);
                EXPECT_FALSE(textOf(*w.work("advanced"), "asks").empty());

                // Shut, it is refused and the day does not pass
                ASSERT_TRUE(w.take("advanced"));
                EXPECT_FALSE(textOf(w.last.site, "refused").empty());
                EXPECT_EQ(w.day(), 3);
                EXPECT_EQ(w.mark("deep"), 0);
            }

            // Two lectures: taught twice, the course is open on the third day
            {
                Walker w(student());

                ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
                ASSERT_TRUE(w.take("lecture"));
                ASSERT_TRUE(w.take("lecture"));

                EXPECT_EQ(w.mark("taught"), 2);

                ASSERT_NE(w.work("advanced"), nullptr);
                EXPECT_EQ(intOf(*w.work("advanced"), "locked"), 0);
                EXPECT_EQ(textOf(*w.work("advanced"), "asks"), "");

                ASSERT_TRUE(w.take("advanced"));
                EXPECT_EQ(textOf(w.last.site, "refused"), "");
                EXPECT_EQ(w.mark("deep"), 1);
                EXPECT_EQ(intOf(w.character, "coin"), 2);
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A course is paid the day it is followed; without the coin it is shut, saying what it costs.
        TEST(story_test, a_tile_charges_its_fee_when_taken)
        {
            MockLogger logger;

            {
                Walker w(student());

                ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

                EXPECT_EQ(intOf(*w.work("lecture"), "locked"), 0);

                ASSERT_TRUE(w.take("lecture"));

                EXPECT_EQ(intOf(w.last.after, "coin"), 4);
                EXPECT_EQ(w.mark("taught"), 1);
            }

            {
                ElementMap poor = student();
                poor["coin"] = ElementType{0};

                Walker w(poor);

                ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

                ASSERT_NE(w.work("lecture"), nullptr);
                EXPECT_EQ(intOf(*w.work("lecture"), "locked"), 1);
                EXPECT_NE(textOf(*w.work("lecture"), "asks").find("COIN"), std::string::npos) << textOf(*w.work("lecture"), "asks");

                ASSERT_TRUE(w.act("tile", "lecture"));

                EXPECT_FALSE(textOf(w.state.run, "refused").empty());
                EXPECT_EQ(w.day(), 1);
                EXPECT_EQ(intOf(w.character, "coin"), 0);
                EXPECT_EQ(w.mark("taught"), 0);
                EXPECT_TRUE(w.state.record.empty());
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A tile that may be taken once is gone once taken.
        TEST(story_test, a_tile_taken_most_times_leaves)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_NE(w.work("once"), nullptr);

            ASSERT_TRUE(w.take("once"));

            EXPECT_EQ(w.day(), 2);
            EXPECT_EQ(w.work("once"), nullptr);

            // Asked again, it is refused
            ASSERT_TRUE(w.take("once"));
            EXPECT_FALSE(textOf(w.last.site, "refused").empty());
            EXPECT_EQ(w.day(), 2);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // Without a coin there is still a day's work for him, every day.
        TEST(story_test, a_day_always_offers_a_free_tile)
        {
            MockLogger logger;

            ElementMap poor = student();
            poor["coin"] = ElementType{0};

            Walker w(poor);

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

            for (int day = 1; day <= 4; ++day)
            {
                ASSERT_EQ(w.day(), day);

                std::string open;

                for (const auto& row : w.last.works)
                {
                    if (open.empty() and intOf(row, "locked") == 0)
                        open = textOf(row, "id");
                }

                ASSERT_FALSE(open.empty()) << "day " << day;

                ASSERT_TRUE(w.act(open));
            }

            EXPECT_EQ(textOf(w.state.run, "mode"), "done");
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // The days are worth the months: four days for four months, a month falls due at the end of
        // each. The rules count both, from the day; a look counts nothing.
        TEST(story_test, months_fall_due_as_days_pass)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

            EXPECT_EQ(intOf(w.state.run, "months"), 4);
            EXPECT_EQ(intOf(w.state.run, "days"), 4);
            EXPECT_EQ(intOf(w.state.run, "monthsDue"), 0);
            EXPECT_EQ(intOf(w.state.run, "monthsRun"), 0);

            for (int day = 1; day <= 4; ++day)
            {
                ASSERT_TRUE(w.take("library"));

                EXPECT_EQ(intOf(w.state.run, "monthsDue"), 1) << "day " << day;
                EXPECT_EQ(intOf(w.state.run, "monthsRun"), day) << "day " << day;

                // A look runs no month, and moves neither count
                ASSERT_TRUE(w.ask("look"));

                EXPECT_EQ(intOf(w.last.state.run, "monthsDue"), 1);
                EXPECT_EQ(intOf(w.last.state.run, "monthsRun"), day);
            }

            // An act that ends no day has no month due
            Walker idle(student());

            ASSERT_TRUE(idle.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_TRUE(idle.take("library"));
            EXPECT_EQ(intOf(idle.state.run, "monthsDue"), 1);

            ASSERT_TRUE(idle.act("leave"));
            EXPECT_EQ(intOf(idle.state.run, "monthsDue"), 0);
            EXPECT_EQ(intOf(idle.state.run, "monthsRun"), 1);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // The last day ends the scene, and nothing of it is kept as a record.
        TEST(story_test, the_last_day_ends_the_scene)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

            for (int day = 1; day <= 3; ++day)
            {
                ASSERT_TRUE(w.take("library"));
                EXPECT_EQ(textOf(w.state.run, "mode"), "site");
            }

            ASSERT_TRUE(w.take("library"));

            EXPECT_EQ(textOf(w.state.run, "mode"), "done");
            EXPECT_EQ(w.screen(), "done");
            EXPECT_EQ(textOf(w.last.recipe, "zone"), "");
            EXPECT_EQ(textOf(w.state.run, "result"), "over");
            EXPECT_EQ(w.mark("revised"), 4);

            // No walk of a zone was written to him
            for (const auto& [key, value] : w.character)
                EXPECT_NE(key.rfind("walked.", 0), 0u) << key;

            // Done, nothing more is taken
            ASSERT_TRUE(w.take("library"));
            EXPECT_FALSE(textOf(w.state.run, "refused").empty());
            EXPECT_EQ(w.mark("revised"), 4);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // Leaving is asked twice, and what the scene gave and took stays as it is.
        TEST(story_test, leaving_is_asked_twice_and_keeps_what_was_given)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_TRUE(w.take("lecture"));
            ASSERT_EQ(w.day(), 2);

            ASSERT_TRUE(w.act("leave"));

            EXPECT_EQ(textOf(w.state.run, "mode"), "site");
            EXPECT_EQ(intOf(w.state.run, "leaving"), 1);
            EXPECT_EQ(w.day(), 2);

            ASSERT_TRUE(w.act("leave"));

            EXPECT_EQ(textOf(w.state.run, "mode"), "done");
            EXPECT_EQ(w.screen(), "done");
            EXPECT_EQ(textOf(w.state.run, "result"), "left");

            // The coin paid stays paid, the lecture stays heard
            EXPECT_EQ(intOf(w.last.after, "coin"), 4);
            EXPECT_EQ(intOf(w.character, "coin"), 4);
            EXPECT_EQ(w.mark("taught"), 1);
        }
    }
}
