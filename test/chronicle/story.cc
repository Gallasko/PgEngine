#include "stdafx.h"

#include <algorithm>
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

                // The card up, "story.<id>" for a story's own, "" when none
                std::string cardId() const { return screen() == "event" ? textOf(last.card, "id") : std::string(); }

                // The card up answered with its first answer that can be given
                bool answerFirst()
                {
                    for (const auto& o : last.options)
                    {
                        if (intOf(o, "locked") == 0)
                            return act("option", textOf(o, "id"));
                    }

                    ADD_FAILURE() << "no answer can be given to " << cardId();
                    return false;
                }

                // The story's chance meeting, when one came at the start of the day, answered
                void settle()
                {
                    for (int guard = 0; guard < 4 and cardId() == "story.rumour"; ++guard)
                        answerFirst();
                }

                // Every card that waits answered with its first answer, till the tiles are back, the
                // scene is over, or the card `until` is up
                void answerAll(const std::string& until = "")
                {
                    for (int guard = 0; guard < 8 and screen() == "event" and (until.empty() or cardId() != until); ++guard)
                        answerFirst();
                }

                // A tile taken, then the chance meeting answered if one came
                bool spend(const std::string& tile)
                {
                    const bool ok = take(tile);

                    settle();

                    return ok;
                }

                int tilesTaken() const
                {
                    int n = 0;

                    for (const auto& step : state.record)
                    {
                        if (textOf(step, "kind") == "tile")
                            ++n;
                    }

                    return n;
                }

                const ElementMap* option(const std::string& id) const { return byId(last.options, id); }

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

            // The page's own form of it, `tile.<id>` as the act's kind, is the same (a chance meeting
            // at the start of the second day answered first, and written as a step of its own)
            w.settle();
            ASSERT_TRUE(w.take("library"));

            EXPECT_EQ(w.day(), 3);
            EXPECT_EQ(w.mark("revised"), 2);
            EXPECT_EQ(w.tilesTaken(), 2);
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
                ASSERT_TRUE(w.spend("library"));

                EXPECT_NE(w.work("early"), nullptr);
                EXPECT_EQ(w.work("advanced"), nullptr);

                ASSERT_TRUE(w.take("library"));

                // The third day's cards, answered first
                w.answerAll();

                EXPECT_EQ(w.day(), 3);
                EXPECT_EQ(w.screen(), "site");
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
                ASSERT_TRUE(w.spend("lecture"));
                ASSERT_TRUE(w.take("lecture"));
                w.answerAll();

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

            ASSERT_TRUE(w.spend("once"));

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
        // Without a coin there is still a day's work for him, every day the tiles are offered (the
        // fourth is the examination's: having heard no lecture, he is sent away before it).
        TEST(story_test, a_day_always_offers_a_free_tile)
        {
            MockLogger logger;

            ElementMap poor = student();
            poor["coin"] = ElementType{0};

            Walker w(poor);

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

            for (int day = 1; day <= 3; ++day)
            {
                w.answerAll();

                ASSERT_EQ(w.day(), day);
                ASSERT_EQ(w.screen(), "site");

                std::string open;

                for (const auto& row : w.last.works)
                {
                    if (open.empty() and intOf(row, "locked") == 0)
                        open = textOf(row, "id");
                }

                ASSERT_FALSE(open.empty()) << "day " << day;

                ASSERT_TRUE(w.act(open));
            }

            EXPECT_EQ(w.cardId(), "story.sent_away");

            w.answerAll();

            EXPECT_EQ(textOf(w.state.run, "mode"), "done");
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // The days are worth the months: four days for four months, a month falls due at the end of
        // each, the examination's day as well. The rules count both, from the day; a look counts
        // nothing, nor does an answer to a card.
        TEST(story_test, months_fall_due_as_days_pass)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

            EXPECT_EQ(intOf(w.state.run, "months"), 4);
            EXPECT_EQ(intOf(w.state.run, "days"), 4);
            EXPECT_EQ(intOf(w.state.run, "monthsDue"), 0);
            EXPECT_EQ(intOf(w.state.run, "monthsRun"), 0);

            const char* tiles[] = {"lecture", "library", "library"};

            for (int day = 1; day <= 4; ++day)
            {
                // The fourth day is the examination
                if (day < 4)
                {
                    ASSERT_TRUE(w.take(tiles[day - 1]));
                }
                else
                {
                    w.answerAll("story.exam");
                    ASSERT_EQ(w.cardId(), "story.exam");
                    ASSERT_TRUE(w.act("option", "plain"));
                }

                EXPECT_EQ(intOf(w.state.run, "monthsDue"), 1) << "day " << day;
                EXPECT_EQ(intOf(w.state.run, "monthsRun"), day) << "day " << day;

                // A look runs no month, and moves neither count
                ASSERT_TRUE(w.ask("look"));

                EXPECT_EQ(intOf(w.last.state.run, "monthsDue"), 1);
                EXPECT_EQ(intOf(w.last.state.run, "monthsRun"), day);

                // A card answered ends no day
                if (w.screen() == "event" and w.cardId() != "story.exam")
                {
                    ASSERT_TRUE(w.answerFirst());

                    EXPECT_EQ(intOf(w.state.run, "monthsDue"), 0) << "day " << day;
                    EXPECT_EQ(intOf(w.state.run, "monthsRun"), day) << "day " << day;
                }
            }

            EXPECT_EQ(textOf(w.state.run, "mode"), "done");

            // An act that ends no day has no month due
            Walker idle(student());

            ASSERT_TRUE(idle.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_TRUE(idle.take("library"));
            EXPECT_EQ(intOf(idle.state.run, "monthsDue"), 1);

            idle.settle();
            ASSERT_TRUE(idle.act("leave"));
            EXPECT_EQ(intOf(idle.state.run, "monthsDue"), 0);
            EXPECT_EQ(intOf(idle.state.run, "monthsRun"), 1);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // The last day ends the scene (its examination, here), and nothing of it is kept as a record.
        TEST(story_test, the_last_day_ends_the_scene)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

            const char* tiles[] = {"lecture", "library", "library"};

            for (int day = 1; day <= 3; ++day)
            {
                w.answerAll();
                ASSERT_TRUE(w.take(tiles[day - 1]));
                EXPECT_EQ(textOf(w.state.run, "mode"), "site");
            }

            w.answerAll("story.exam");
            ASSERT_EQ(w.cardId(), "story.exam");
            ASSERT_TRUE(w.act("option", "plain"));

            EXPECT_EQ(textOf(w.state.run, "mode"), "done");
            EXPECT_EQ(w.screen(), "done");
            EXPECT_EQ(textOf(w.last.recipe, "zone"), "");
            EXPECT_EQ(textOf(w.state.run, "result"), "pass");
            EXPECT_EQ(w.mark("revised"), 2);

            // No walk of a zone was written to him
            for (const auto& [key, value] : w.character)
                EXPECT_NE(key.rfind("walked.", 0), 0u) << key;

            // Done, nothing more is taken
            ASSERT_TRUE(w.take("library"));
            EXPECT_FALSE(textOf(w.state.run, "refused").empty());
            EXPECT_EQ(w.mark("revised"), 2);
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
            ASSERT_TRUE(w.spend("lecture"));
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

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // Leaving a story he can drop out of makes him a drop-out of it, keeping what it gave.
        TEST(story_test, leaving_a_class_story_is_a_dropout)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_TRUE(w.spend("lecture"));

            EXPECT_EQ(intOf(w.character, "dropout.trial"), 0);

            ASSERT_TRUE(w.act("leave"));
            EXPECT_EQ(intOf(w.character, "dropout.trial"), 0);

            ASSERT_TRUE(w.act("leave"));

            EXPECT_EQ(textOf(w.state.run, "result"), "left");
            EXPECT_EQ(intOf(w.last.after, "dropout.trial"), 1);
            EXPECT_EQ(intOf(w.character, "coin"), 4);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // The third day's card comes on the third day, with its four answers; an answer is not a
        // day: the card shuts and the day's tiles are there again.
        TEST(story_test, a_card_opens_on_its_day)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_TRUE(w.spend("lecture"));

            EXPECT_EQ(w.screen(), "site");

            ASSERT_TRUE(w.take("lecture"));

            EXPECT_EQ(w.day(), 3);
            ASSERT_EQ(w.screen(), "event");
            EXPECT_EQ(w.cardId(), "story.crossroads");
            EXPECT_EQ(textOf(w.state.run, "event"), "story.crossroads");
            EXPECT_FALSE(textOf(w.last.card, "title").empty());
            ASSERT_EQ(w.last.options.size(), 4u);

            for (const char* id : {"a", "b", "c", "d"})
            {
                ASSERT_NE(w.option(id), nullptr) << id;
                EXPECT_EQ(intOf(*w.option(id), "locked"), 0) << id;
                EXPECT_FALSE(textOf(*w.option(id), "label").empty()) << id;
            }

            // A tile is refused while the card waits
            ASSERT_TRUE(w.take("library"));
            EXPECT_FALSE(textOf(w.last.site, "refused").empty());
            EXPECT_EQ(w.mark("revised"), 0);

            ASSERT_TRUE(w.act("option", "b"));

            EXPECT_EQ(textOf(w.last.site, "refused"), "");
            EXPECT_EQ(w.mark("b"), 1);
            EXPECT_EQ(w.mark("a"), 0);
            EXPECT_EQ(w.screen(), "site");
            EXPECT_EQ(textOf(w.state.run, "event"), "");
            EXPECT_EQ(w.day(), 3);
            EXPECT_NE(w.work("library"), nullptr);
            EXPECT_EQ(intOf(*w.work("library"), "locked"), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A card that waits on a mark opens at the start of the first day it is met, before that
        // day's own card; each opens once.
        TEST(story_test, a_card_opens_when_its_mark_is_met)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_TRUE(w.spend("library"));

            EXPECT_EQ(w.screen(), "site");

            ASSERT_TRUE(w.take("library"));

            EXPECT_EQ(w.day(), 3);
            EXPECT_EQ(w.mark("revised"), 2);
            ASSERT_EQ(w.cardId(), "story.noticed");

            ASSERT_TRUE(w.answerFirst());
            ASSERT_EQ(w.cardId(), "story.crossroads");

            ASSERT_TRUE(w.answerFirst());
            EXPECT_EQ(w.screen(), "site");
            EXPECT_EQ(w.day(), 3);

            // The next day neither comes again
            ASSERT_TRUE(w.take("library"));

            EXPECT_EQ(w.day(), 4);
            EXPECT_NE(w.cardId(), "story.noticed");
            EXPECT_NE(w.cardId(), "story.crossroads");
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A story walks no zone: the chance meetings of its days are its own, never a place's.
        TEST(story_test, story_events_come_from_the_story)
        {
            MockLogger logger;

            const std::vector<std::string> own = {"story.rumour", "story.crossroads", "story.noticed", "story.sent_away", "story.exam"};
            int rumours = 0;

            for (int s = 1; s <= 50; ++s)
            {
                Walker w(student());

                w.seed = s;

                ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

                for (int guard = 0; guard < 16 and textOf(w.state.run, "mode") != "done"; ++guard)
                {
                    if (w.screen() == "event")
                    {
                        const std::string id = w.cardId();

                        EXPECT_NE(std::find(own.begin(), own.end(), id), own.end()) << "seed " << s << ": " << id;

                        if (id == "story.rumour")
                            ++rumours;

                        ASSERT_TRUE(w.answerFirst());
                    }
                    else
                    {
                        ASSERT_TRUE(w.take(w.mark("taught") == 0 ? "lecture" : "library"));
                    }
                }

                EXPECT_EQ(textOf(w.state.run, "mode"), "done") << "seed " << s;
            }

            EXPECT_GT(rumours, 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // The examination counts his wits once and his days in the library twice: 12 and 2 make 16,
        // a pass, and the pass gives what it gives.
        TEST(story_test, a_test_scores_his_stats_and_marks)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_TRUE(w.spend("lecture"));
            ASSERT_TRUE(w.spend("library"));
            w.answerAll();
            ASSERT_TRUE(w.take("library"));

            EXPECT_EQ(w.mark("revised"), 2);

            w.answerAll("story.exam");
            ASSERT_EQ(w.cardId(), "story.exam");
            EXPECT_EQ(w.day(), 4);
            EXPECT_EQ(w.last.options.size(), 1u);

            ASSERT_TRUE(w.act("option", "plain"));

            EXPECT_EQ(intOf(w.state.run, "score.exam"), 16);
            EXPECT_EQ(textOf(w.state.run, "result"), "pass");
            EXPECT_EQ(textOf(w.state.run, "mode"), "done");
            EXPECT_EQ(intOf(w.character, "trialled"), 1);
            EXPECT_EQ(intOf(w.character, "dropout.trial"), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // Answering from the old tome is there only for who holds it, and counts what it taught in
        // place of his wits.
        TEST(story_test, an_answer_appears_for_who_holds_its_flag)
        {
            MockLogger logger;

            auto toTheExam = [](Walker& w)
            {
                ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
                ASSERT_TRUE(w.spend("lecture"));
                ASSERT_TRUE(w.spend("library"));
                w.answerAll();
                ASSERT_TRUE(w.take("library"));
                w.answerAll("story.exam");
                ASSERT_EQ(w.cardId(), "story.exam");
            };

            {
                Walker w(student());

                toTheExam(w);

                EXPECT_NE(w.option("plain"), nullptr);
                EXPECT_EQ(w.option("tome"), nullptr);

                // Not listed, it cannot be given either
                ASSERT_TRUE(w.act("option", "tome"));
                EXPECT_FALSE(textOf(w.last.site, "refused").empty());
                EXPECT_EQ(w.cardId(), "story.exam");
            }

            {
                ElementMap reader = student();
                reader["tome"] = ElementType{1};
                reader["lore"] = ElementType{8};

                Walker w(reader);

                toTheExam(w);

                EXPECT_NE(w.option("plain"), nullptr);
                ASSERT_NE(w.option("tome"), nullptr);
                EXPECT_EQ(intOf(*w.option("tome"), "locked"), 0);

                ASSERT_TRUE(w.act("option", "tome"));

                // lore 8 in place of int 12, and the library's 2 twice
                EXPECT_EQ(intOf(w.state.run, "score.exam"), 12);
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A score below the pass is a fail: he drops out, and keeps what the days gave him.
        TEST(story_test, a_poor_score_is_a_dropout)
        {
            MockLogger logger;

            ElementMap dull = student();
            dull["int"] = ElementType{6};

            Walker w(dull);

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));
            ASSERT_TRUE(w.spend("once"));
            ASSERT_TRUE(w.spend("lecture"));
            w.answerAll();
            ASSERT_TRUE(w.take("library"));
            w.answerAll("story.exam");
            ASSERT_EQ(w.cardId(), "story.exam");

            ASSERT_TRUE(w.act("option", "plain"));

            EXPECT_EQ(intOf(w.state.run, "score.exam"), 8);
            EXPECT_EQ(textOf(w.state.run, "result"), "fail");
            EXPECT_EQ(textOf(w.state.run, "mode"), "done");
            EXPECT_EQ(intOf(w.last.after, "dropout.trial"), 1);
            EXPECT_EQ(intOf(w.last.after, "trialled"), 0);

            // The dean's renown and the lecture's coin are as the days left them
            EXPECT_EQ(intOf(w.last.after, "renown"), 1);
            EXPECT_EQ(intOf(w.last.after, "coin"), 4);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // Who has heard no lecture is sent away on the examination's day: a drop-out, and the
        // examination never comes.
        TEST(story_test, too_few_courses_and_he_is_sent_away)
        {
            MockLogger logger;
            Walker w(student());

            ASSERT_TRUE(w.ask("enter", WalkAct{"story", "trial", 0}));

            std::vector<std::string> seen;

            for (int day = 1; day <= 3; ++day)
            {
                for (int guard = 0; guard < 4 and w.screen() == "event"; ++guard)
                {
                    seen.push_back(w.cardId());
                    ASSERT_TRUE(w.answerFirst());
                }

                ASSERT_TRUE(w.take("library"));
            }

            EXPECT_EQ(w.day(), 4);
            ASSERT_EQ(w.cardId(), "story.sent_away");

            ASSERT_TRUE(w.act("option", "go"));

            EXPECT_EQ(textOf(w.state.run, "mode"), "done");
            EXPECT_EQ(w.screen(), "done");
            EXPECT_EQ(textOf(w.state.run, "result"), "dropout");
            EXPECT_EQ(intOf(w.last.after, "dropout.trial"), 1);
            EXPECT_EQ(intOf(w.state.run, "sat.exam"), 0);
            EXPECT_EQ(std::find(seen.begin(), seen.end(), "story.exam"), seen.end());

            // Over, nothing more is answered
            ASSERT_TRUE(w.act("option", "plain"));
            EXPECT_FALSE(textOf(w.last.site, "refused").empty());
        }
    }
}
