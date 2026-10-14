#include "stdafx.h"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/rulevm.h"
#include "Core/walk.h"

#include "ECS/entitysystem.h"

#include "mocklogger.h"

using namespace chronicle;

// The walks outside the town, as the rules play them: rules/expedition.pg asked through
// Rules::walk, with the walk as it stands handed in and taken back at every step, the way the
// Life screen does it (the scripts are copied beside the test binary).
namespace pg
{
    namespace test
    {
        namespace
        {
            constexpr const char* Root = "rules";

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

            // The marks of a kind for a thing, added up
            int marked(const RecordList& marks, const std::string& kind, const std::string& id)
            {
                int sum = 0;

                for (const auto& mark : marks)
                {
                    if (textOf(mark, "kind") == kind and textOf(mark, "id") == id)
                        sum += intOf(mark, "n");
                }

                return sum;
            }

            // A boy who has helped at the hedges: the Hedgerows are open to him
            ElementMap gatherer()
            {
                return {{"str", ElementType{6}}, {"dex", ElementType{6}}, {"int", ElementType{6}}, {"vit", ElementType{40}}, {"vitmax", ElementType{40}},
                        {"coin", ElementType{0}}, {"rations", ElementType{12}}};
            }

            // A youth who can fight: the Hollow Woods are open to him
            ElementMap hunter()
            {
                return {{"str", ElementType{11}}, {"dex", ElementType{10}}, {"int", ElementType{9}}, {"vit", ElementType{70}}, {"vitmax", ElementType{70}},
                        {"arms", ElementType{2}}, {"coin", ElementType{5}}, {"rations", ElementType{10}}, {"remedy", ElementType{2}}, {"bow_learned", ElementType{1}}};
            }

            // Someone whose world has studied a place that often (3 is its knowledge 2, 6 its 3,
            // 10 its 4): what the place opens a level at a time is open to him
            ElementMap knowing(ElementMap who, const std::string& zone, int studies)
            {
                who["know.visit." + zone] = ElementType{1};
                who["know.study." + zone] = ElementType{studies};

                return who;
            }

            // One who walks: the rules, his character and his walk, each answer taken as the Life
            // screen takes it (the stats become his, the walk becomes the walk as it stands)
            struct Walker
            {
                EntitySystem ecs;
                Rules rules;
                ElementMap character;
                float age = 14.0f;
                WalkState state;
                std::vector<WalkRecipe> recipes;
                WalkQueue queue;
                RecordList walks;
                RuleWalk last;
                int seed = 100;

                explicit Walker(ElementMap who) : character(std::move(who))
                {
                    EXPECT_TRUE(rules.load(&ecs, Root)) << (rules.errors.empty() ? "" : rules.errors.front());

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

                std::string screen() const { return textOf(last.site, "screen"); }

                // An event that comes up is answered with its first option that can be taken
                void answer()
                {
                    for (int guard = 0; guard < 4 and screen() == "event"; ++guard)
                    {
                        std::string option;

                        for (const auto& o : last.options)
                        {
                            if (option.empty() and intOf(o, "locked") == 0)
                                option = textOf(o, "id");
                        }

                        ASSERT_FALSE(option.empty());
                        act("option", option);
                    }
                }

                // A fight handed to his stance, to its end and back to the woods
                void fight()
                {
                    answer();

                    ASSERT_EQ(screen(), "fight");

                    for (int turn = 0; turn < 80 and textOf(last.duel, "over").empty(); ++turn)
                        act("step");

                    ASSERT_FALSE(textOf(last.duel, "over").empty());

                    act("back");
                    answer();
                }

                // The walk just ended, kept as the Life screen keeps it
                void keep(const std::string& id)
                {
                    WalkRecipe recipe;
                    recipe.id = id;
                    recipe.fields = last.recipe;
                    recipe.steps = last.state.record;
                    recipe.marks = last.state.marks;

                    recipes.push_back(recipe);
                    state = WalkState{};
                }
            };
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // He arrives: a week of days before him, the field work of the place, and no beast.
        TEST(expedition_test, a_walk_begins_with_its_days)
        {
            MockLogger logger;
            Walker w(gatherer());

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hedgerows", 0}));

            EXPECT_TRUE(w.state.under());
            EXPECT_EQ(w.screen(), "site");
            EXPECT_EQ(intOf(w.state.run, "ap"), 7);
            EXPECT_EQ(intOf(w.state.run, "months"), 1);

            // His Vitality counts five in the field
            EXPECT_EQ(intOf(w.state.run, "hp"), 40);
            EXPECT_EQ(intOf(w.state.run, "hpMax"), 40);

            // The place is visited from his first step: the world knows it at one of four
            EXPECT_EQ(intOf(w.character, "know.visit.hedgerows"), 1);
            EXPECT_EQ(textOf(w.last.site, "knowledge"), "ZONE KNOWLEDGE 1 OF 4");
            EXPECT_EQ(textOf(w.last.site, "daysLeft"), "7");

            // Three things to do on a first visit, and no more
            EXPECT_TRUE(w.last.seeks.empty());
            EXPECT_EQ(w.last.works.size(), 3u);
            ASSERT_NE(byId(w.last.works, "gather.berries"), nullptr);
            ASSERT_NE(byId(w.last.works, "gather.firewood"), nullptr);
            ASSERT_NE(byId(w.last.works, "study"), nullptr);

            // A cost is said before it is paid, with its unit
            EXPECT_EQ(textOf(*byId(w.last.works, "gather.berries"), "costText"), "1 AP");
            EXPECT_EQ(textOf(*byId(w.last.works, "study"), "costText"), "2 AP");

            // No walk under an unknown name
            Walker lost(gatherer());
            MockLogger quiet;

            EXPECT_FALSE(lost.rules.walk(14.0f, lost.character, "Aldren", "enter", 1, WalkState{}, WalkAct{"", "nowhere", 0}, {}, {}, WalkQueue{}, "", lost.last));
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // What he learned as a boy is his in the field, where he can camp: a snare is a day for a
        // ration, the herbal makes a remedy of two wortleaf where he stands, who says the office
        // rests a little better, and who knows the roads is a day sooner there. Who learned none
        // of it has none of it, and nobody camps at the Hedgerows.
        TEST(expedition_test, a_boys_trades_are_his_in_the_field)
        {
            MockLogger logger;

            // Never taught: no tile for it, and asking is refused
            {
                Walker w(knowing(gatherer(), "millpond", 3));

                ASSERT_TRUE(w.ask("enter", WalkAct{"", "millpond", 0}));

                ASSERT_NE(byId(w.last.works, "rest"), nullptr);
                EXPECT_EQ(byId(w.last.works, "snare"), nullptr);
                EXPECT_EQ(byId(w.last.works, "brew"), nullptr);

                const size_t steps = w.state.record.size();

                ASSERT_TRUE(w.act("snare"));
                EXPECT_EQ(w.state.record.size(), steps);
                EXPECT_FALSE(textOf(w.last.site, "refused").empty());

                ASSERT_TRUE(w.act("brew"));
                EXPECT_EQ(w.state.record.size(), steps);
                EXPECT_FALSE(textOf(w.last.site, "refused").empty());
            }

            // Taught, but at the Hedgerows: no camp, so no rest and no trade, whatever is known of them
            {
                ElementMap boy = knowing(gatherer(), "hedgerows", 10);
                boy["snare_set"] = ElementType{1};
                boy["herbal"] = ElementType{1};

                Walker w(boy);

                ASSERT_TRUE(w.ask("enter", WalkAct{"", "hedgerows", 0}));

                EXPECT_EQ(byId(w.last.works, "rest"), nullptr);
                EXPECT_EQ(byId(w.last.works, "snare"), nullptr);
                EXPECT_EQ(byId(w.last.works, "brew"), nullptr);

                const size_t steps = w.state.record.size();

                ASSERT_TRUE(w.act("rest"));
                EXPECT_EQ(w.state.record.size(), steps);
                EXPECT_FALSE(textOf(w.last.site, "refused").empty());

                ASSERT_TRUE(w.act("snare"));
                EXPECT_EQ(w.state.record.size(), steps);
            }

            // A snare: a day of the stay, a ration
            {
                ElementMap trapper = knowing(gatherer(), "millpond", 3);
                trapper["snare_set"] = ElementType{1};

                Walker w(trapper);

                ASSERT_TRUE(w.ask("enter", WalkAct{"", "millpond", 0}));
                ASSERT_NE(byId(w.last.works, "snare"), nullptr);
                EXPECT_EQ(textOf(*byId(w.last.works, "snare"), "costText"), "1 AP");

                const int rations = intOf(w.character, "rations");
                const int days = intOf(w.state.run, "ap");

                ASSERT_TRUE(w.act("snare"));

                EXPECT_EQ(intOf(w.character, "rations"), rations + 1);
                EXPECT_EQ(intOf(w.state.run, "ap"), days - 1);
                ASSERT_FALSE(w.state.record.empty());
                EXPECT_EQ(textOf(w.state.record.back(), "kind"), "snare");
            }

            // The herbal: shut without the leaf, then two wortleaf for a remedy, and no day spent
            {
                ElementMap herbalist = knowing(gatherer(), "millpond", 3);
                herbalist["herbal"] = ElementType{1};

                Walker w(herbalist);

                ASSERT_TRUE(w.ask("enter", WalkAct{"", "millpond", 0}));
                ASSERT_NE(byId(w.last.works, "brew"), nullptr);
                EXPECT_EQ(intOf(*byId(w.last.works, "brew"), "locked"), 1);

                const size_t steps = w.state.record.size();

                ASSERT_TRUE(w.act("brew"));
                EXPECT_EQ(w.state.record.size(), steps);
                EXPECT_FALSE(textOf(w.last.site, "refused").empty());

                w.character["wortleaf"] = ElementType{3};

                const int days = intOf(w.state.run, "ap");

                ASSERT_TRUE(w.act("brew"));

                EXPECT_EQ(intOf(w.character, "wortleaf"), 1);
                EXPECT_EQ(intOf(w.character, "remedy"), 1);
                EXPECT_EQ(intOf(w.state.run, "ap"), days);
                EXPECT_EQ(w.state.record.size(), steps + 1);
            }

            // A rest eats a ration; the office said over his fire gives two more
            {
                auto rested = [](ElementMap who) {
                    who["vit"] = ElementType{10};

                    Walker w(knowing(std::move(who), "millpond", 3));

                    EXPECT_TRUE(w.ask("enter", WalkAct{"", "millpond", 0}));

                    const int before = intOf(w.state.run, "hp");
                    const int rations = intOf(w.character, "rations");

                    EXPECT_TRUE(w.act("rest"));
                    EXPECT_EQ(intOf(w.character, "rations"), rations - 1);

                    return intOf(w.state.run, "hp") - before;
                };

                ElementMap sacristan = gatherer();
                sacristan["anselm_trust"] = ElementType{1};

                EXPECT_EQ(rested(sacristan), rested(gatherer()) + 2);
            }

            // The roads known: a day off the way there, and so off the way home
            {
                Walker plain(hunter());
                ASSERT_TRUE(plain.ask("enter", WalkAct{"", "hollow", 0}));

                ElementMap carter = hunter();
                carter["roads_known"] = ElementType{1};

                Walker w(carter);
                ASSERT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));

                EXPECT_EQ(intOf(w.state.run, "travel"), intOf(plain.state.run, "travel") - 1);
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // Every day spent is a line of the record, with what it cost and what it gave.
        TEST(expedition_test, a_day_is_written_to_the_record)
        {
            MockLogger logger;
            Walker w(gatherer());

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hedgerows", 0}));
            ASSERT_TRUE(w.act("gather", "berries"));
            w.answer();

            const int berries = marked(w.state.marks, "gain", "berries");

            EXPECT_GE(berries, 1);
            EXPECT_EQ(intOf(w.character, "berries"), berries);
            EXPECT_LE(intOf(w.state.run, "ap"), 6);

            ASSERT_FALSE(w.state.record.empty());
            EXPECT_EQ(textOf(w.state.record[0], "kind"), "gather");
            EXPECT_EQ(intOf(w.state.record[0], "ap"), 1);
            EXPECT_NE(textOf(w.state.record[0], "figure").find("1 AP"), std::string::npos);
            EXPECT_NE(textOf(w.state.record[0], "figure").find("BERRIES"), std::string::npos);

            // What is for the town is his to hold like the rest, until a work takes it
            ASSERT_TRUE(w.act("gather", "firewood"));
            w.answer();

            EXPECT_GE(intOf(w.character, "firewood"), 1);
            EXPECT_EQ(w.character.count("store.firewood"), 0u);

            // A study teaches the world
            ASSERT_TRUE(w.act("study"));
            w.answer();

            EXPECT_GE(intOf(w.character, "know.study.hedgerows"), 1);

            // What cannot be paid is refused, and nothing is written
            const size_t steps = w.state.record.size();

            while (intOf(w.state.run, "ap") >= 2 and w.screen() == "site")
            {
                ASSERT_TRUE(w.act("study"));
                w.answer();
            }

            EXPECT_GT(w.state.record.size(), steps);

            if (w.screen() == "site" and intOf(w.state.run, "ap") == 1)
            {
                const size_t before = w.state.record.size();

                ASSERT_TRUE(w.act("study"));
                EXPECT_EQ(w.state.record.size(), before);
                EXPECT_FALSE(textOf(w.last.site, "refused").empty());
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // Leaving is asked twice; the walk is then over, and what it was is what a record keeps.
        TEST(expedition_test, turning_home_is_asked_twice)
        {
            MockLogger logger;
            Walker w(gatherer());

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hedgerows", 0}));
            ASSERT_TRUE(w.act("gather", "berries"));
            w.answer();

            ASSERT_TRUE(w.act("leave"));

            // The first press says what is lost, and nothing is done
            EXPECT_EQ(w.screen(), "site");
            EXPECT_EQ(intOf(w.state.run, "leaving"), 1);
            EXPECT_NE(textOf(w.last.site, "leave").find("lost"), std::string::npos);
            EXPECT_TRUE(textOf(w.last.recipe, "zone").empty());

            // Something else done lets go of it
            ASSERT_TRUE(w.act("gather", "berries"));
            w.answer();
            EXPECT_EQ(intOf(w.state.run, "leaving"), 0);

            ASSERT_TRUE(w.act("leave"));
            ASSERT_TRUE(w.act("leave"));

            EXPECT_EQ(w.screen(), "done");
            EXPECT_EQ(textOf(w.last.recipe, "zone"), "hedgerows");
            EXPECT_EQ(intOf(w.last.recipe, "steps"), static_cast<int>(w.state.record.size()));
            EXPECT_EQ(intOf(w.last.recipe, "months"), 1);
            EXPECT_EQ(intOf(w.character, "walked.hedgerows"), 1);
            EXPECT_EQ(textOf(w.state.record.back(), "kind"), "leave");

            // And the walk does nothing more
            const size_t steps = w.state.record.size();

            ASSERT_TRUE(w.act("gather", "berries"));
            EXPECT_EQ(w.state.record.size(), steps);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // The stay ends with its last day, whether or not he turned for home.
        TEST(expedition_test, the_last_day_ends_the_stay)
        {
            MockLogger logger;
            Walker w(gatherer());

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hedgerows", 0}));

            for (int day = 0; day < 20 and w.screen() != "done"; ++day)
            {
                ASSERT_TRUE(w.act("gather", "berries"));
                w.answer();
            }

            EXPECT_EQ(w.screen(), "done");
            EXPECT_EQ(intOf(w.state.run, "ap"), 0);
            EXPECT_EQ(intOf(w.last.recipe, "ap"), 7);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A beast sought is fought: the fight costs days by its turns, and its line says what it took.
        TEST(expedition_test, a_fight_costs_days_by_its_turns)
        {
            MockLogger logger;
            Walker w(hunter());

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));

            // On a first visit one wolf alone is all the wood shows him: the pack, the boar and
            // the rare beast come with the place known better
            const ElementMap* lone = byId(w.last.seeks, "wolf");

            ASSERT_NE(lone, nullptr);
            EXPECT_EQ(w.last.seeks.size(), 1u);
            EXPECT_EQ(byId(w.last.seeks, "wolf*pack"), nullptr);
            EXPECT_EQ(byId(w.last.seeks, "boar"), nullptr);
            EXPECT_EQ(byId(w.last.seeks, "stag"), nullptr);

            // Known 4 of 4: the pack is cheaper to find than one wolf alone, and the rare beast is listed
            {
                Walker known(knowing(hunter(), "hollow", 10));

                ASSERT_TRUE(known.ask("enter", WalkAct{"", "hollow", 0}));

                const ElementMap* pack = byId(known.last.seeks, "wolf*pack");
                const ElementMap* alone = byId(known.last.seeks, "wolf");
                const ElementMap* stag = byId(known.last.seeks, "stag");

                ASSERT_NE(pack, nullptr);
                ASSERT_NE(alone, nullptr);
                ASSERT_NE(stag, nullptr);

                EXPECT_LT(intOf(*pack, "cost"), intOf(*alone, "cost"));
                EXPECT_EQ(intOf(*pack, "count"), 3);
                EXPECT_EQ(intOf(*stag, "locked"), 0);
            }

            const int before = intOf(w.state.run, "ap");
            const int seekCost = intOf(*lone, "cost");

            ASSERT_TRUE(w.act("seek", "wolf", 1));
            w.answer();

            ASSERT_EQ(w.screen(), "fight");
            EXPECT_EQ(w.state.foes.size(), 1u);
            EXPECT_EQ(textOf(w.last.duel, "turn"), "TURN 1");

            // What he may do is what anyone can: strike, defend, take a remedy while he holds
            // one, flee. He has learned nothing else, and nothing else is offered
            ASSERT_EQ(w.last.menu.size(), 4u);
            EXPECT_EQ(textOf(w.last.menu[0], "id"), "strike");
            EXPECT_EQ(textOf(w.last.menu[1], "name"), "Defend");
            EXPECT_EQ(textOf(w.last.menu[2], "id"), "remedy");
            EXPECT_EQ(textOf(w.last.menu[3], "name"), "Flee");
            EXPECT_EQ(textOf(w.last.preview, "id"), "strike");
            EXPECT_FALSE(textOf(w.last.preview, "deals").empty());

            // What he fights is said of each beast: how it stands, and what it means to do to him
            ASSERT_EQ(w.last.targets.size(), 1u);
            EXPECT_EQ(textOf(w.last.targets[0], "life"), "UNHURT");
            EXPECT_EQ(intOf(w.last.targets[0], "percent"), 100);
            EXPECT_FALSE(textOf(w.last.targets[0], "intent").empty());
            EXPECT_FALSE(textOf(w.last.duel, "about").empty());

            // Looking at another choice changes nothing but what is said
            ASSERT_TRUE(w.act("pick", "guard"));
            EXPECT_EQ(textOf(w.last.preview, "id"), "guard");
            EXPECT_EQ(textOf(w.last.preview, "title"), "Defend");
            EXPECT_EQ(textOf(w.last.duel, "turn"), "TURN 1");

            // What he has not learned cannot be done, whoever asks
            ASSERT_TRUE(w.act("pick", "tech.heavy"));
            ASSERT_TRUE(w.act("do"));
            EXPECT_EQ(textOf(w.last.duel, "turn"), "TURN 1");
            EXPECT_EQ(textOf(w.last.duel, "refused"), "LEARNED AT THE KEEP");

            ASSERT_TRUE(w.act("pick", "strike"));

            w.fight();

            // The fight's line: after the seek's, with the days it cost
            ASSERT_GE(w.state.record.size(), 2u);

            const ElementMap* line = nullptr;

            for (const auto& step : w.state.record)
            {
                if (textOf(step, "kind") == "fight")
                    line = &step;
            }

            ASSERT_NE(line, nullptr);
            EXPECT_GE(intOf(*line, "ap"), 1);
            EXPECT_EQ(textOf(*line, "target"), "wolf");
            EXPECT_EQ(intOf(w.character, "know.fights.wolf"), 1);
            EXPECT_EQ(intOf(w.character, "fought.wolf"), 1);

            EXPECT_LE(intOf(w.state.run, "ap"), before - seekCost - intOf(*line, "ap"));

            // His Vitality in the field is his own: the same points at home, and never nothing
            EXPECT_GE(intOf(w.character, "vit"), 1);
            EXPECT_EQ(intOf(w.character, "vit"), intOf(w.state.run, "hp"));
            EXPECT_EQ(intOf(w.state.run, "hpMax"), intOf(w.character, "vitmax"));
            EXPECT_EQ(w.character.count("scratches"), 0u);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // What a life has learned is what its fights offer: the Keep's blows to its sworn men, the
        // Collegium's workings to its scholars, a trick of the streets to who knows them.
        TEST(expedition_test, a_fight_offers_what_he_has_learned)
        {
            MockLogger logger;

            auto learned = [](ElementMap who) {
                Walker w(std::move(who));

                EXPECT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));
                w.act("seek", "wolf", 1);
                w.answer();

                std::vector<std::string> ids;

                for (const auto& m : w.last.menu)
                {
                    if (textOf(m, "group") == "Learned")
                        ids.push_back(textOf(m, "id"));
                }

                return ids;
            };

            EXPECT_TRUE(learned(hunter()).empty());

            ElementMap knight = hunter();
            knight["keep_oath"] = ElementType{1};
            EXPECT_EQ(learned(knight), (std::vector<std::string>{"tech.heavy"}));

            knight["veteran"] = ElementType{1};
            EXPECT_EQ(learned(knight), (std::vector<std::string>{"tech.heavy", "tech.sweep"}));

            ElementMap mage = hunter();
            mage["collegium"] = ElementType{1};
            mage["orin_known"] = ElementType{1};
            EXPECT_EQ(learned(mage), (std::vector<std::string>{"tech.firebolt", "tech.binding"}));

            ElementMap urchin = hunter();
            urchin["street_known"] = ElementType{1};
            EXPECT_EQ(learned(urchin), (std::vector<std::string>{"tech.feint"}));

            // What the wooden blade taught a boy, the edge or the point, comes first: it was first
            ElementMap edge = hunter();
            edge["slash_learned"] = ElementType{1};
            EXPECT_EQ(learned(edge), (std::vector<std::string>{"tech.slash"}));

            // It is a better strike and no more: it costs no breath, and he is told of none
            {
                Walker w(edge);

                EXPECT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));
                w.act("seek", "wolf", 1);
                w.answer();

                for (const auto& m : w.last.menu)
                {
                    if (textOf(m, "id") == "tech.slash")
                        EXPECT_EQ(textOf(m, "cost"), "");
                }

                EXPECT_EQ(textOf(w.last.duel, "breath"), "");

                // Used a turn, it is his again the next: nothing to wait for
                ASSERT_TRUE(w.act("pick", "tech.slash"));
                ASSERT_TRUE(w.act("do"));
                EXPECT_EQ(textOf(w.last.duel, "refused"), "");

                ASSERT_TRUE(w.act("pick", "tech.slash"));
                ASSERT_TRUE(w.act("do"));
                EXPECT_EQ(textOf(w.last.duel, "refused"), "");
            }

            ElementMap point = hunter();
            point["thrust_learned"] = ElementType{1};
            point["keep_oath"] = ElementType{1};
            EXPECT_EQ(learned(point), (std::vector<std::string>{"tech.thrust", "tech.heavy"}));

            // The levy teaches its boys the heavy cut before any oath, and the smuggler his one trick
            ElementMap levied = hunter();
            levied["levy_served"] = ElementType{1};
            EXPECT_EQ(learned(levied), (std::vector<std::string>{"tech.heavy"}));

            ElementMap runner = hunter();
            runner["smuggler_runner"] = ElementType{1};
            EXPECT_EQ(learned(runner), (std::vector<std::string>{"tech.sand"}));

            // A working is counted from his Intelligence: a clever man's firebolt is worth his study
            mage["int"] = ElementType{16};

            Walker w(mage);

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));
            ASSERT_TRUE(w.act("seek", "wolf", 1));
            w.answer();
            ASSERT_EQ(w.screen(), "fight");

            ASSERT_TRUE(w.act("pick", "tech.firebolt"));
            EXPECT_EQ(textOf(w.last.preview, "title"), "Firebolt");
            EXPECT_EQ(textOf(w.last.preview, "deals"), "12\xE2\x80\x93" "15");   // 5-8, and half of 16, less the wolf's guard of 1

            // The binding word deals nothing: the beast loses the move it meant to make
            ASSERT_TRUE(w.act("pick", "tech.binding"));
            EXPECT_EQ(textOf(w.last.preview, "deals"), "Nothing");

            const int hp = intOf(w.state.run, "hp");
            const int breath = intOf(w.state.fight, "breath");

            ASSERT_TRUE(w.act("do"));

            EXPECT_EQ(intOf(w.state.run, "hp"), hp);
            EXPECT_EQ(intOf(w.state.fight, "breath"), breath - 2);
            EXPECT_EQ(textOf(w.last.duel, "turn"), "TURN 2");
            EXPECT_EQ(intOf(w.state.foes[0], "hp"), intOf(w.state.foes[0], "hpMax"));
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // What a beast means to do is said by the move's name alone. What the move does to him is
        // said once it has been used on him, or once he has watched the beast: not before.
        TEST(expedition_test, a_move_is_known_once_it_was_used_on_him)
        {
            MockLogger logger;

            ElementMap boy = knowing(gatherer(), "hedgerows", 10);

            Walker w(boy);
            w.age = 10.0f;

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "millpond", 0}));
            ASSERT_TRUE(w.act("seek", "rat", 1));
            w.answer();
            ASSERT_EQ(w.screen(), "fight");

            // No rat is quicker than a boy
            EXPECT_NE(textOf(w.last.duel, "quick").find("HE ACTS FIRST"), std::string::npos);

            // He defends until it has bitten at him once: the bite is a name and nothing else till then
            bool sawUnknown = false;

            for (int turn = 0; turn < 12 and intOf(w.character, "know.move.rat.bite") == 0 and textOf(w.last.duel, "over").empty(); ++turn)
            {
                ASSERT_EQ(w.last.targets.size(), 1u);

                if (textOf(w.last.targets[0], "intent") == "BITE")
                {
                    sawUnknown = true;
                    EXPECT_EQ(textOf(w.last.targets[0], "moveName"), "Bite");
                    EXPECT_FALSE(textOf(w.last.targets[0], "moveLine").empty());
                    EXPECT_EQ(intOf(w.last.targets[0], "moveKnown"), 0);
                    EXPECT_TRUE(textOf(w.last.targets[0], "moveEffect").empty());

                    // And what is coming is counted, not weighed
                    EXPECT_EQ(textOf(w.last.duel, "warn"), "1 BLOW COMING");
                }
                else
                {
                    // What draws no blood is plain to see
                    EXPECT_EQ(intOf(w.last.targets[0], "moveKnown"), 1);
                }

                ASSERT_TRUE(w.act("move", "guard"));
            }

            EXPECT_TRUE(sawUnknown);
            ASSERT_EQ(intOf(w.character, "know.move.rat.bite"), 1);

            // From here on it says what it takes, in this fight and in the next: the world knows it
            for (int turn = 0; turn < 6 and textOf(w.last.duel, "over").empty(); ++turn)
            {
                if (textOf(w.last.targets[0], "intent") == "BITE")
                {
                    EXPECT_EQ(intOf(w.last.targets[0], "moveKnown"), 1);
                    EXPECT_NE(textOf(w.last.targets[0], "moveEffect").find("vitality if it lands"), std::string::npos);
                    EXPECT_NE(textOf(w.last.duel, "warn").find("IF ALL LAND"), std::string::npos);
                }

                ASSERT_TRUE(w.act("move", "guard"));
            }

            // A button pressed is the move done: every row of the menu carries what its gloss says
            for (const auto& m : w.last.menu)
                EXPECT_FALSE(textOf(m, "text").empty()) << textOf(m, "id");
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A field work says what its gloss needs: a line on it, what it costs in words, what it gives
        TEST(expedition_test, a_field_work_says_what_it_costs_and_gives)
        {
            MockLogger logger;

            Walker w(gatherer());

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hedgerows", 0}));
            ASSERT_FALSE(w.last.works.empty());

            auto work = [&](const std::string& id) -> ElementMap {
                for (const auto& row : w.last.works)
                {
                    if (textOf(row, "id") == id)
                        return row;
                }

                ADD_FAILURE() << "no work " << id;

                return {};
            };

            for (const auto& row : w.last.works)
            {
                EXPECT_FALSE(textOf(row, "about").empty()) << textOf(row, "id");
                EXPECT_FALSE(textOf(row, "costs").empty()) << textOf(row, "id");
                EXPECT_FALSE(textOf(row, "gives").empty()) << textOf(row, "id");
            }

            EXPECT_EQ(textOf(work("gather.berries"), "costs"), "1 day of the stay");
            EXPECT_EQ(textOf(work("gather.berries"), "gives"), "1\xE2\x80\x93" "2 berries, to sell");
            EXPECT_EQ(textOf(work("gather.firewood"), "costs"), "2 days of the stay");
            EXPECT_EQ(textOf(work("gather.firewood"), "gives"), "1\xE2\x80\x93" "3 firewood, for the town");

            // A study says what the next level opens
            EXPECT_NE(textOf(work("study"), "gives").find("to knowledge 2"), std::string::npos);
            EXPECT_NE(textOf(work("study"), "gives").find("which opens exploring the nearby pond"), std::string::npos);
            EXPECT_NE(textOf(w.last.site, "knowledgeNote").find("exploring the nearby pond"), std::string::npos);

            // And so does a rest, where he can camp
            Walker camper(knowing(gatherer(), "millpond", 3));

            ASSERT_TRUE(camper.ask("enter", WalkAct{"", "millpond", 0}));

            const ElementMap* rest = byId(camper.last.works, "rest");

            ASSERT_NE(rest, nullptr);
            EXPECT_NE(textOf(*rest, "costs").find("1 ration of"), std::string::npos);
            EXPECT_NE(textOf(*rest, "gives").find("Vitality"), std::string::npos);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A place opens a little at a time: three things to do on a first visit, one more at each
        // level of its knowledge, and at the fourth the next place. What is not open is refused.
        TEST(expedition_test, a_place_opens_a_level_at_a_time)
        {
            MockLogger logger;

            auto works = [](int studies) {
                Walker w(knowing(gatherer(), "hedgerows", studies));
                std::vector<std::string> ids;

                EXPECT_TRUE(w.ask("enter", WalkAct{"", "hedgerows", 0}));

                for (const auto& row : w.last.works)
                    ids.push_back(textOf(row, "id"));

                return ids;
            };

            using Ids = std::vector<std::string>;

            EXPECT_EQ(works(0), (Ids{"gather.berries", "gather.firewood", "study"}));
            EXPECT_EQ(works(3), (Ids{"gather.berries", "gather.firewood", "study"}));
            EXPECT_EQ(works(6), (Ids{"gather.berries", "gather.firewood", "gather.wortleaf", "study"}));
            EXPECT_EQ(works(10), (Ids{"gather.berries", "gather.firewood", "gather.wortleaf", "gather.hazel", "study"}));

            // Known 3 of 4, the study says the fourth opens the next place
            {
                Walker w(knowing(gatherer(), "hedgerows", 6));

                ASSERT_TRUE(w.ask("enter", WalkAct{"", "hedgerows", 0}));
                ASSERT_NE(byId(w.last.works, "study"), nullptr);
                EXPECT_NE(textOf(*byId(w.last.works, "study"), "gives").find("the way to the Hollow Woods"), std::string::npos);
            }

            // What is not open is refused, and nothing is written
            {
                Walker w(gatherer());

                ASSERT_TRUE(w.ask("enter", WalkAct{"", "hedgerows", 0}));
                ASSERT_TRUE(w.act("gather", "wortleaf"));

                EXPECT_TRUE(w.state.record.empty());
                EXPECT_FALSE(textOf(w.last.site, "refused").empty());
                EXPECT_EQ(intOf(w.state.run, "ap"), 7);
            }

            // A pack is not sought before the wood is known 4 of 4
            {
                Walker w(hunter());

                ASSERT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));

                const int days = intOf(w.state.run, "ap");

                ASSERT_TRUE(w.act("seek", "wolf", 3));

                EXPECT_EQ(w.screen(), "site");
                EXPECT_FALSE(textOf(w.last.site, "refused").empty());
                EXPECT_EQ(intOf(w.state.run, "ap"), days);
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A blow that found nothing is marked in the account, his or the beast's, and no other line is
        TEST(expedition_test, a_missed_blow_is_marked_in_the_account)
        {
            MockLogger logger;

            int his = 0;
            int theirs = 0;

            for (int seed = 100; seed < 1300 and (his == 0 or theirs == 0); seed += 100)
            {
                ElementMap boy = knowing(gatherer(), "hedgerows", 10);

                Walker w(boy);
                w.age = 10.0f;
                w.seed = seed;

                ASSERT_TRUE(w.ask("enter", WalkAct{"", "millpond", 0}));
                ASSERT_TRUE(w.act("seek", "rat", 1));
                w.answer();

                if (w.screen() != "fight")
                    continue;

                for (int turn = 0; turn < 12 and textOf(w.last.duel, "over").empty(); ++turn)
                    ASSERT_TRUE(w.act("move", "strike"));

                for (const auto& line : w.state.account)
                {
                    const std::string text = textOf(line, "text");
                    const bool miss = text.find(": a miss") != std::string::npos or text.find("steps clear") != std::string::npos;

                    EXPECT_EQ(intOf(line, "missed") > 0, miss) << text;

                    if (miss and textOf(line, "by") == "hero")
                        ++his;

                    if (miss and textOf(line, "by") == "foe")
                        ++theirs;
                }
            }

            EXPECT_GT(his, 0);
            EXPECT_GT(theirs, 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A retreat ends the fight at once: the days stay written, and nothing is carried off.
        TEST(expedition_test, a_retreat_keeps_the_days_and_leaves_the_loot)
        {
            MockLogger logger;
            Walker w(knowing(hunter(), "hollow", 10));   // The wood known 4 of 4: its packs are found

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));
            ASSERT_TRUE(w.act("seek", "wolf", 3));
            w.answer();

            ASSERT_EQ(w.screen(), "fight");
            EXPECT_EQ(w.state.foes.size(), 3u);

            // Of a pack of three, one hangs back while two stand
            int holding = 0;

            for (const auto& t : w.last.targets)
            {
                if (textOf(t, "intent") == "HANG BACK")
                    ++holding;
            }

            EXPECT_EQ(holding, 1);

            ASSERT_TRUE(w.act("step"));
            ASSERT_TRUE(w.act("pick", "retreat"));
            ASSERT_TRUE(w.act("do"));

            EXPECT_EQ(textOf(w.last.duel, "over"), "fled");

            ASSERT_TRUE(w.act("back"));
            w.answer();

            const ElementMap* line = nullptr;

            for (const auto& step : w.state.record)
            {
                if (textOf(step, "kind") == "fight")
                    line = &step;
            }

            ASSERT_NE(line, nullptr);
            EXPECT_EQ(textOf(*line, "tone"), "loss");
            EXPECT_GE(intOf(*line, "ap"), 1);
            EXPECT_EQ(marked(w.state.marks, "kill", "wolf"), 0);
            EXPECT_EQ(marked(w.state.marks, "loot", "pelts"), 0);
            EXPECT_EQ(intOf(w.character, "pelts"), 0);
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // The same dice make the same walk: a walk is asked with its own.
        TEST(expedition_test, the_same_dice_make_the_same_walk)
        {
            MockLogger logger;

            auto walked = [](int seed) {
                Walker w(knowing(hunter(), "hollow", 10));
                w.seed = seed;

                EXPECT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));
                w.act("seek", "wolf", 3);
                w.fight();
                w.act("gather", "ashwood");
                w.answer();

                std::string told;

                for (const auto& step : w.state.record)
                    told += textOf(step, "text") + "|" + textOf(step, "figure") + ";";

                return told;
            };

            EXPECT_EQ(walked(500), walked(500));
            EXPECT_FALSE(walked(500).empty());
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A record walked again is a fold over its steps: what it took is taken, what it gathered
        // is gathered, what its beasts drop is rolled anew, and no fight is fought.
        TEST(expedition_test, a_record_is_walked_again_by_its_steps)
        {
            MockLogger logger;
            Walker w(knowing(hunter(), "hollow", 3));   // The wood known 2 of 4: he camps there

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));
            ASSERT_TRUE(w.act("seek", "wolf", 1));
            w.fight();
            ASSERT_TRUE(w.act("gather", "ashwood"));
            w.answer();
            ASSERT_TRUE(w.act("rest"));
            w.answer();
            ASSERT_TRUE(w.act("leave"));
            ASSERT_TRUE(w.act("leave"));
            ASSERT_EQ(w.screen(), "done");

            const int gathered = marked(w.state.marks, "gain", "ashwood");
            const int eaten = marked(w.state.marks, "use", "rations");
            const int total = intOf(w.last.recipe, "hp");

            ASSERT_GE(gathered, 1);
            ASSERT_GE(eaten, 1);

            w.keep("r1");

            // The records page: his record, with what a queue of it would take and bring
            w.walks = {{{"zone", ElementType{std::string("hollow")}}, {"listed", ElementType{1}}, {"locked", ElementType{0}}, {"months", ElementType{3}}, {"until", ElementType{std::string()}}}};
            w.queue.recipe = "r1";
            w.queue.count = 3;

            ASSERT_TRUE(w.ask("records"));

            ASSERT_EQ(w.last.shelf.size(), 1u);
            EXPECT_EQ(textOf(w.last.shelf[0], "id"), "r1");
            EXPECT_EQ(textOf(w.last.shelf[0], "state"), "kept");
            EXPECT_EQ(textOf(w.last.form, "recipe"), "r1");
            EXPECT_EQ(intOf(w.last.form, "months"), 9);
            EXPECT_EQ(textOf(w.last.form, "total"), "3 TIMES \xC2\xB7 9 MO");
            EXPECT_TRUE(textOf(w.last.form, "blocked").empty());
            EXPECT_FALSE(w.last.stops.empty());

            // He sets out: what the record eats is taken as he leaves
            const int rations = intOf(w.character, "rations");
            const int ash = intOf(w.character, "ashwood");
            const int vit = intOf(w.character, "vit");

            ASSERT_TRUE(w.ask("begin", {}, "r1"));
            EXPECT_TRUE(w.last.blocked.empty());
            EXPECT_EQ(intOf(w.character, "rations"), rations - eaten);

            // He is home: the fold
            ASSERT_TRUE(w.ask("term", {}, "r1"));

            EXPECT_EQ(intOf(w.character, "ashwood"), ash + gathered);
            EXPECT_EQ(intOf(w.character, "walked.hollow"), 2);
            EXPECT_EQ(intOf(w.character, "know.fights.wolf"), 2);
            EXPECT_GE(intOf(w.character, "pelts"), 1);   // A wolf always leaves its pelt
            ASSERT_FALSE(w.last.entries.empty());

            // What the walk cost him it costs again, and never his life
            EXPECT_GE(intOf(w.character, "vit"), 1);
            EXPECT_LE(intOf(w.character, "vit"), 70);

            if (total < 0)
                EXPECT_LE(intOf(w.character, "vit"), vit);

            // With nothing to eat the record cannot be walked: the bag says what it lacks
            w.character["rations"] = ElementType{0};

            ASSERT_TRUE(w.ask("begin", {}, "r1"));
            EXPECT_FALSE(w.last.blocked.empty());
            EXPECT_NE(w.last.blocked.find("ration"), std::string::npos);
            EXPECT_EQ(intOf(w.character, "rations"), 0);

            ASSERT_TRUE(w.ask("records"));
            EXPECT_FALSE(textOf(w.last.form, "blocked").empty());
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // What is known is the world's, and it changes what a walk costs and shows.
        TEST(expedition_test, knowledge_makes_a_beast_cheaper_to_find_and_easier_to_read)
        {
            MockLogger logger;
            Walker w(hunter());

            ASSERT_TRUE(w.ask("enter", WalkAct{"", "hollow", 0}));

            const int unknown = intOf(*byId(w.last.seeks, "wolf"), "cost");

            EXPECT_EQ(byId(w.last.works, "rest"), nullptr);

            // Three studies: the zone is known two of four, a wolf is found a day sooner, and
            // what the second level opens is open (a camp); the rare beast waits for the fourth
            for (int i = 0; i < 3; ++i)
            {
                ASSERT_TRUE(w.act("study"));
                w.answer();
            }

            ASSERT_TRUE(w.ask("look"));

            EXPECT_EQ(textOf(w.last.site, "knowledge"), "ZONE KNOWLEDGE 2 OF 4");
            EXPECT_NE(byId(w.last.works, "rest"), nullptr);
            EXPECT_EQ(byId(w.last.seeks, "stag"), nullptr);
            EXPECT_LT(intOf(*byId(w.last.seeks, "wolf"), "cost"), unknown);

            // Watched twice, a beast is known two of four
            ASSERT_TRUE(w.act("observe", "wolf"));
            ASSERT_TRUE(w.act("observe", "wolf"));

            EXPECT_EQ(intOf(w.character, "know.observe.wolf"), 2);
            EXPECT_NE(textOf(*byId(w.last.seeks, "wolf"), "rank").find("KNOWN 2 OF 4"), std::string::npos);

            // And what is known of it is shown in a fight: its vitality in figures
            ASSERT_TRUE(w.act("seek", "wolf", 1));
            w.answer();

            if (w.screen() == "fight")
            {
                ASSERT_FALSE(w.last.targets.empty());
                EXPECT_NE(textOf(w.last.targets[0], "life").find(" OF "), std::string::npos);
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // A walk is a row of the Life page: the Adventure group, its months up front, locked until
        // what opens it is done.
        TEST(expedition_test, a_walk_is_a_row_of_the_life_page)
        {
            MockLogger logger;
            Walker w(gatherer());

            std::vector<RuleActivity> activities;

            ASSERT_TRUE(w.rules.activities(8.0f, w.character, activities));

            const RuleActivity* hedgerows = nullptr;
            const RuleActivity* pond = nullptr;

            for (const auto& a : activities)
            {
                if (textOf(a.fields, "id") == "walk.hedgerows")
                    hedgerows = &a;

                if (textOf(a.fields, "id") == "walk.millpond")
                    pond = &a;

                // No task stands before the first walk: its tile takes him there
                EXPECT_NE(textOf(a.fields, "id"), "hedges");
            }

            ASSERT_NE(hedgerows, nullptr);
            ASSERT_NE(pond, nullptr);

            EXPECT_EQ(textOf(hedgerows->fields, "group"), "Adventure");
            EXPECT_EQ(textOf(hedgerows->fields, "zone"), "hedgerows");
            EXPECT_EQ(intOf(hedgerows->fields, "months"), 1);

            // The first walk asks nothing of him: listed, and open
            EXPECT_EQ(intOf(hedgerows->fields, "listed"), 1);
            EXPECT_EQ(intOf(hedgerows->fields, "locked"), 0);
            EXPECT_TRUE(hedgerows->requires.empty());

            // The next one has no row until he has explored the pond, a task of his life: one place
            // at a time. Walked, or known 4 of 4 without it, is not enough
            EXPECT_EQ(intOf(pond->fields, "listed"), 0);

            auto row = [&](const std::string& id) -> const RuleActivity* {
                for (const auto& a : activities)
                {
                    if (textOf(a.fields, "id") == id)
                        return &a;
                }

                return nullptr;
            };

            // That task has no row before the Hedgerows are known 2 of 4
            ASSERT_NE(row("pond"), nullptr);
            EXPECT_EQ(intOf(row("pond")->fields, "listed"), 0);

            w.character["walked.hedgerows"] = ElementType{1};
            w.character["know.study.hedgerows"] = ElementType{10};
            ASSERT_TRUE(w.rules.activities(8.0f, w.character, activities));

            ASSERT_NE(row("walk.millpond"), nullptr);
            EXPECT_EQ(intOf(row("walk.millpond")->fields, "listed"), 0);

            // Known 2 of 4 (and more): the task is his, a month on the Life page, and it is not a walk;
            // what it gives is the world's
            ASSERT_NE(row("pond"), nullptr);
            EXPECT_EQ(intOf(row("pond")->fields, "listed"), 1);
            EXPECT_EQ(intOf(row("pond")->fields, "locked"), 0);
            EXPECT_EQ(intOf(row("pond")->fields, "months"), 1);
            EXPECT_EQ(textOf(row("pond")->fields, "group"), "Work");
            EXPECT_TRUE(textOf(row("pond")->fields, "zone").empty());
            ASSERT_EQ(row("pond")->gains.size(), 1u);
            EXPECT_EQ(textOf(row("pond")->gains[0], "stat"), "know.found.millpond");

            // Done, the world knows the pond (`know.found.millpond`): the task is offered no more
            w.character["know.found.millpond"] = ElementType{1};
            ASSERT_TRUE(w.rules.activities(8.0f, w.character, activities));

            ASSERT_NE(row("pond"), nullptr);
            EXPECT_EQ(intOf(row("pond")->fields, "listed"), 0);

            for (const auto& a : activities)
            {
                // Explored: it is listed, and waits for his tenth year
                if (textOf(a.fields, "id") == "walk.millpond")
                {
                    EXPECT_EQ(intOf(a.fields, "listed"), 1);
                    EXPECT_EQ(intOf(a.fields, "locked"), 1);
                }
            }

            for (const auto& a : activities)
            {
                if (textOf(a.fields, "id") == "walk.hedgerows")
                    EXPECT_EQ(intOf(a.fields, "locked"), 0);

                // The Hollow Woods are three months: the way there, the stay and the way back
                if (textOf(a.fields, "id") == "walk.hollow")
                    EXPECT_EQ(intOf(a.fields, "months"), 3);
            }
        }

        // ----------------------------------------------------------------------------------------
        // ---------------------------        Test separator        -------------------------------
        // ----------------------------------------------------------------------------------------
        // What a walk brings for the town is his, and ends with him: at his death only what the
        // place he chooses still asks of it is left there, and counts toward its next level.
        TEST(expedition_test, what_a_place_asks_can_be_left_to_it)
        {
            MockLogger logger;
            Walker w(hunter());

            w.character["town_known"] = ElementType{1};
            w.character["seen.lodge"] = ElementType{1};
            w.character["coin"] = ElementType{30};
            w.character["firewood"] = ElementType{32};
            w.character["ashwood"] = ElementType{4};
            w.character["stone"] = ElementType{9};

            auto lodge = [&](RuleTown& town) -> const RulePlace* {
                EXPECT_TRUE(w.rules.town(14.0f, w.character, town)) << (w.rules.errors.empty() ? "" : w.rules.errors.front());

                for (const auto& place : town.places)
                {
                    if (textOf(place.fields, "id") == "lodge")
                        return &place;
                }

                return nullptr;
            };

            auto left = [](const RulePlace* place, const std::string& stat) {
                for (const auto& l : place->leaves)
                {
                    if (textOf(l, "stat") == stat)
                        return intOf(l, "amount");
                }

                return 0;
            };

            // Its first level asks 20 firewood and 10 ash wood: no more than it asks, no more than
            // he holds, and nothing of what it does not ask
            RuleTown town;
            const RulePlace* place = lodge(town);
            ASSERT_NE(place, nullptr);

            EXPECT_EQ(left(place, "firewood"), 20);
            EXPECT_EQ(left(place, "ashwood"), 4);
            EXPECT_EQ(left(place, "stone"), 0);
            EXPECT_EQ(textOf(place->fields, "leaveLine"), "20 firewood, 4 ash wood");
            EXPECT_TRUE(intOf(place->fields, "nextLocked"));   // Six ash wood short

            // What a life before left it counts toward what it asks, as coin does
            w.character["fund.lodge.firewood"] = ElementType{15};
            w.character["fund.lodge.ashwood"] = ElementType{10};

            RuleTown after;
            place = lodge(after);
            ASSERT_NE(place, nullptr);

            EXPECT_EQ(left(place, "firewood"), 5);
            EXPECT_EQ(left(place, "ashwood"), 0);
            EXPECT_FALSE(intOf(place->fields, "nextLocked"));

            bool firewoodAsked = false;

            for (const auto& gap : place->gaps)
            {
                if (textOf(gap, "stat") == "firewood")
                {
                    firewoodAsked = true;
                    EXPECT_EQ(intOf(gap, "needed"), 5);
                }

                EXPECT_NE(textOf(gap, "stat"), "ashwood");
            }

            EXPECT_TRUE(firewoodAsked);

            // And the work that raises it takes the same from him
            std::vector<RuleActivity> activities;
            ASSERT_TRUE(w.rules.activities(14.0f, w.character, activities));

            for (const auto& a : activities)
            {
                if (textOf(a.fields, "id") != "raise.lodge.1")
                    continue;

                int firewood = -1;
                int ashwood = -1;

                for (const auto& c : a.costs)
                {
                    if (textOf(c, "stat") == "firewood")
                        firewood = intOf(c, "amount");

                    if (textOf(c, "stat") == "ashwood")
                        ashwood = intOf(c, "amount");
                }

                EXPECT_EQ(firewood, 5);
                EXPECT_EQ(ashwood, -1);
            }
        }
    }
}
