#include "lifescene.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "logger.h"

#include "ECS/entitysystem.h"
#include "ECS/callable.h"
#include "2D/position.h"
#include "UI/prefab.h"
#include "UI/sizer.h"
#include "UI/prefabloader.h"
#include "UI/prefabbuilder.h"
#include "Systems/gamefacts.h"

#include "UI/label.h"
#include "UI/panel.h"
#include "UI/button.h"
#include "UI/gloss.h"
#include "UI/tabs.h"
#include "UI/eventlog.h"
#include "UI/activityrow.h"
#include "UI/progressrule.h"
#include "UI/resourceledger.h"

using namespace pg;

// The walks outside the town, as the Life scene plays them: this file is LifeScene's, the part
// of it that is a walk's. A walk is a row of the Life page like any other (the Adventure group,
// rules/activitytable.pg): begun, it opens the on-site screen over the page instead of running
// its months, the days of the stay are chosen one at a time there, a fight takes that screen's
// place and gives it back, and once he turns for home the walk's months run as any work's do.
// What he did is kept as a record, and the Adventure page walks it again, once or in a queue,
// with no day chosen and no fight fought.
//
// The scene computes nothing of it: every figure and every word is rules/expedition.pg's, asked
// through Rules::walk. The walk as it stands is the save's (LifeSave::walk), handed to the
// script and taken back at every step, so a walk closed in its middle opens where it stood.
namespace chronicle
{
    namespace
    {
        constexpr const char * const DOM = "Chronicle.Walk";

        constexpr const char * const ActivitiesList = "life.activities";

        // The lists of the walk's screens and of the Adventure page, by the id their events carry
        constexpr const char * const SeeksList = "walk.seeks";
        constexpr const char * const WorksList = "walk.works";
        constexpr const char * const MenuList = "walk.menu";
        constexpr const char * const TargetsList = "walk.targets";
        constexpr const char * const RecordsList = "walk.records";

        // Their buttons, by tag
        constexpr const char * const WalkTag = "walk.";
        constexpr const char * const SeekTag = "walk.seek";
        constexpr const char * const ObserveTag = "walk.observe";
        constexpr const char * const WorkTag = "walk.work";
        constexpr const char * const LeaveTag = "walk.leave";
        constexpr const char * const OptionTag = "walk.option.";   // Followed by the option's place on the card
        constexpr const char * const ActTag = "walk.act";
        constexpr const char * const HandTag = "walk.hand";
        constexpr const char * const FewerTag = "walk.fewer";
        constexpr const char * const MoreTag = "walk.more";
        constexpr const char * const StopTag = "walk.stop";
        constexpr const char * const OnceTag = "walk.once";
        constexpr const char * const ManyTag = "walk.many";
        constexpr const char * const LastTag = "walk.last";

        // The Adventure page's pieces (life.yaml and its two smaller pages)
        constexpr const char * const AdventureName = "adventure";
        constexpr const char * const RecordsName = "records";

        const std::string WalkActivity = "walk.";          // An activity that is a walk: followed by its zone
        const std::string ZoneRow = "zone:";               // A row of the records for a zone with no record yet
        const std::string RecordGloss = "record/";
        constexpr int MaxOptions = 3;                      // The buttons an event's card has
        constexpr int MaxChips = 6;                        // The places the order of a turn has
        constexpr int MostWalks = 20;                      // The longest queue the form asks for
        constexpr int SettlePasses = 3;                    // A screen just built is filled again that many passes, once its layouts hold their pieces
        constexpr int TrialAge = 15;                       // --walk: the age of the life it opens on, when the zone asks no more,
        constexpr int TrialParts = 5;                      // and what each of his parts is given over a child's

        // The screens' frame (expedition.yaml, fight.yaml): where the columns start, their fixed
        // widths, and under what width the three stand in one
        constexpr float WalkMargin = 16.0f;
        constexpr float WalkGap = 12.0f;
        constexpr float WalkScroll = 4.0f;         // A column's lane past its panels
        constexpr float SiteTop = 136.0f;
        constexpr float SiteLeft = 300.0f;         // As the Life page's: how he stands and what he holds at the left,
        constexpr float SiteRight = 360.0f;        // what has happened at the right, the choice between them
        constexpr float TakenRoom = 200.0f;        // What he has taken, over the record: the record has the column's rest
        constexpr float FightTop = 118.0f;
        constexpr float FightLeft = 280.0f;
        constexpr float FightRight = 376.0f;
        constexpr float PreviewRoom = 500.0f;      // What the choice would do, over the account: the account has the column's rest
        constexpr float NarrowSiteTop = 160.0f;    // In one column the head's figures stand under its title, and the strip's caption takes two lines
        constexpr float NarrowFightTop = 150.0f;
        constexpr float NarrowFigure = 180.0f;     // The days left, or the turn, at the head's right
        constexpr float NarrowTitle = 120.0f;      // The least a title keeps beside it
        constexpr float NarrowBelow = 1000.0f;
        constexpr float PanelChrome = 83.0f;       // A panel with a heading: 16 + the 51 head block + 16
        constexpr float LeastLog = 160.0f;
        constexpr float LeastColumn = 200.0f;
        constexpr float NoteRoom = 64.0f;          // What stands under a log in its panel: a line or two, and the gaps
        constexpr float HeadRoom = 300.0f;         // The figures at the head's right, and air before them

        // What the veil calls when the mouse enters or leaves it: nothing
        struct WalkNoOp {};

        int intOf(const ElementType& v)
        {
            switch (v.type)
            {
            case UnionType::INT:    return v.get<int>();
            case UnionType::FLOAT:  return static_cast<int>(v.get<float>());
            case UnionType::DOUBLE: return static_cast<int>(v.get<double>());
            case UnionType::SIZE_T: return static_cast<int>(v.get<size_t>());
            case UnionType::BOOL:   return v.get<bool>() ? 1 : 0;
            default:                return 0;
            }
        }

        std::string textOf(const ElementMap& map, const std::string& key)
        {
            auto it = map.find(key);

            return it == map.end() ? std::string() : it->second.toString();
        }

        int intOf(const ElementMap& map, const std::string& key)
        {
            auto it = map.find(key);

            return it == map.end() ? 0 : intOf(it->second);
        }

        bool boolOf(const ElementMap& map, const std::string& key)
        {
            auto it = map.find(key);

            return it != map.end() and it->second.type == UnionType::BOOL and it->second.get<bool>();
        }

        std::string upper(std::string s)
        {
            for (char& c : s)
            {
                if (c >= 'a' and c <= 'z')
                    c = static_cast<char>(c - 'a' + 'A');
            }

            return s;
        }

        std::string lower(std::string s)
        {
            for (char& c : s)
            {
                if (c >= 'A' and c <= 'Z')
                    c = static_cast<char>(c - 'A' + 'a');
            }

            return s;
        }

        LogKind kindOf(const std::string& kind)
        {
            if (kind == "gain")      return LogKind::Gain;
            if (kind == "loss")      return LogKind::Loss;
            if (kind == "coin")      return LogKind::Coin;
            if (kind == "time")      return LogKind::Coin;
            if (kind == "milestone") return LogKind::Milestone;

            return LogKind::Note;
        }

        // In a file a piece stands in its panel's body inside a wrap, and the wrap is what the
        // layout holds: to take the piece out of the stack it is the wrap that is hidden
        EntityRef wrapIn(EntityRef body, EntityRef piece)
        {
            if (body.empty() or not body->has<VerticalLayout>())
                return piece;

            for (auto& child : body->get<VerticalLayout>()->entities)
            {
                if (child.id == piece.id)
                    return child;

                if (not child->has<Prefab>())
                    continue;

                auto prefab = child->get<Prefab>();

                if (prefab->namedChildrenIds.count("MainEntity") > 0 and prefab->getEntity("MainEntity").id == piece.id)
                    return child;
            }

            return piece;
        }

        void windowSize(EntitySystem* ecs, float& width, float& height)
        {
            width = 1320.0f;
            height = 1020.0f;

            if (auto window = ecs->getEntity("__MainWindow"); window and window->has<PositionComponent>())
            {
                auto pos = window->get<PositionComponent>();

                if (pos->width > 0.0f and pos->height > 0.0f)
                {
                    width = pos->width;
                    height = pos->height;
                }
            }
        }

        // A node of a screen's file by its name, the ones a layout holds as well
        NodeSpec* childNamed(NodeSpec& page, const std::string& name)
        {
            for (auto& child : page.children)
            {
                if (child.name == name)
                    return &child;

                if (child.kind.rfind("Layout:", 0) == 0)
                {
                    if (NodeSpec* inner = childNamed(child, name))
                        return inner;
                }
            }

            return nullptr;
        }

        void setProp(NodeSpec* node, const std::string& key, float value)
        {
            if (node)
                node->props[key] = value;
        }

        // What a column has of the window's height, under the head
        float columnHeight(bool fighting, bool narrow, float height)
        {
            const float top = narrow ? (fighting ? NarrowFightTop : NarrowSiteTop) : (fighting ? FightTop : SiteTop);

            return std::max(LeastColumn, height - top - WalkMargin);
        }

        // And a log that fills its column: the panel's chrome and the lines under the log taken off
        float logHeight(float column)
        {
            return std::max(LeastLog, column - PanelChrome - NoteRoom);
        }

        // A walk's screen at a window's size, written over what its file is drawn with. Three
        // columns: the two at the sides hold their width and the middle one takes the rest. One
        // column: every panel is as wide as the window leaves it
        void shapeWalk(NodeSpec& page, bool fighting, bool narrow, float width, float height)
        {
            const float tall = columnHeight(fighting, narrow, height);
            const float whole = std::max(LeastColumn, width - 2.0f * WalkMargin);

            setProp(childNamed(page, "veil"), "width", width);
            setProp(childNamed(page, "veil"), "height", height);
            setProp(childNamed(page, "strip"), "width", whole);
            setProp(childNamed(page, "order"), "width", whole);

            for (const char* name : {"kicker", "zoneTitle", "zoneLine", "fightTitle", "quick"})
                setProp(childNamed(page, name), "width", narrow ? whole : std::max(LeastColumn, whole - HeadRoom));

            if (narrow)
            {
                // The big figure keeps the head's right corner: the two lines beside it end before it
                for (const char* name : {"kicker", "zoneTitle", "fightTitle"})
                    setProp(childNamed(page, name), "width", std::max(NarrowTitle, whole - NarrowFigure));

                for (const char* name : {"headNote", "turnNote", "warn"})
                    setProp(childNamed(page, name), "width", whole);

                setProp(childNamed(page, "walkLeft"), "width", whole + WalkScroll);
                setProp(childNamed(page, "walkLeft"), "height", tall);

                for (const char* name : {"statePanel", "seekPanel", "workPanel", "cardPanel", "recordPanel", "bagPanel", "takenPanel", "homePanel",
                                         "heroPanel", "foePanel", "menuPanel", "previewPanel", "accountPanel"})
                    setProp(childNamed(page, name), "width", whole);

                return;
            }

            const float left = fighting ? FightLeft : SiteLeft;
            const float right = fighting ? FightRight : SiteRight;
            const float middle = std::max(LeastColumn, whole - left - right - 2.0f * WalkGap - 2.0f * WalkScroll);

            for (const char* name : {"walkLeft", "walkMiddle", "walkRight"})
                setProp(childNamed(page, name), "height", tall);

            setProp(childNamed(page, "walkMiddle"), "width", middle + WalkScroll);

            for (const char* name : {"cardPanel", "seekPanel", "workPanel", "menuPanel"})
                setProp(childNamed(page, name), "width", middle);

            for (const char* name : {"takenPanel", "recordPanel", "previewPanel", "accountPanel"})
                setProp(childNamed(page, name), "width", right);

            // A log is a panel's child: found in it, not in the page. The record stands under what
            // he has taken, the account under what the choice would do
            if (NodeSpec* holder = childNamed(page, "recordPanel"))
                setProp(childNamed(*holder, "record"), "height", logHeight(tall - TakenRoom));

            if (NodeSpec* holder = childNamed(page, "accountPanel"))
                setProp(childNamed(*holder, "account"), "height", logHeight(tall - PreviewRoom));
        }

        // What a list was filled with, to fill it again only when that changed: a list filled
        // again lets go of what is chosen in it
        std::string signatureOf(const std::vector<ActivityGroup>& groups)
        {
            std::string signature;

            for (const auto& group : groups)
            {
                signature += group.label + "{";

                for (const auto& row : group.rows)
                {
                    signature += row.id + "|" + row.name + "|" + row.rank + "|" + row.each + "|" + row.cost + "|" + row.until + "|" + std::to_string(static_cast<int>(row.state)) + (row.urgent ? "!" : "") + ";";

                    for (const auto& r : row.requirements)
                        signature += r.label + ",";
                }

                signature += "}";
            }

            return signature;
        }

        // A row that cannot be chosen, and why
        void lockRow(ActivityRowSpec& row, const std::string& asks)
        {
            row.state = ActivityState::Locked;

            Requirement req;
            req.label = asks;
            req.met = 0;

            row.requirements.push_back(req);
        }

        std::string ledgerSignature(const std::vector<LedgerRowSpec>& rows)
        {
            std::string signature;

            for (const auto& row : rows)
                signature += row.id + "|" + row.name + "|" + row.value + "|" + row.rate + (row.muted ? "~" : "") + ";";

            return signature;
        }
    }

    // ---- the script, asked with what the save holds ----------------------------------------------------

    int LifeScene::walkDice()
    {
        // A test gives its own, and every cast is the next of them; the game takes the clock's
        ++diceCast;

        if (opt.walkSeed != 0)
            return opt.walkSeed + diceCast;

        const auto now = std::chrono::steady_clock::now().time_since_epoch();

        return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count() % 60013) + diceCast;
    }

    RecordList LifeScene::walkRows() const
    {
        RecordList rows;

        for (const auto& a : activities)
        {
            const std::string zone = textOf(a.fields, "zone");

            if (zone.empty())
                continue;

            rows.push_back({{"zone", ElementType{zone}},
                            {"listed", ElementType{boolOf(a.fields, "listed") ? 1 : 0}},
                            {"locked", ElementType{boolOf(a.fields, "locked") ? 1 : 0}},
                            {"months", ElementType{intOf(a.fields, "months")}},
                            {"until", ElementType{textOf(a.fields, "until")}}});
        }

        return rows;
    }

    bool LifeScene::askWalk(const std::string& ask, const WalkAct& act, const std::string& replay, RuleWalk& out)
    {
        if (rules.walk(save.age, save.character(), save.name, ask, walkDice(), save.walk, act, save.recipes, walkRows(), save.queue, replay, out))
            return true;

        for (const auto& e : rules.errors)
            LOG_ERROR(DOM, e);

        return false;
    }

    // ---- a walk by hand --------------------------------------------------------------------------------

    void LifeScene::beginWalk(const std::string& activity)
    {
        const RuleActivity* at = activityOf(activity);

        if (not at or ended or onWalk() or not save.running.empty())
            return;

        const std::string zone = textOf(at->fields, "zone");

        if (zone.empty() or boolOf(at->fields, "locked") or not boolOf(at->fields, "listed"))
            return;

        RuleWalk answer;

        if (not askWalk("enter", WalkAct{"", zone, 0}, "", answer) or not answer.state.under())
            return;

        // He is there: the months do not run while the days of the stay are chosen
        paused = true;
        sinceMonth = 0.0f;

        walk = answer;
        save.walk = answer.state;

        takeStats(answer.after);
        writeEntries(answer.entries);

        clearChoice();
        leadDue = false;
        seekChosen.clear();
        workChosen.clear();

        showWalk();

        publish();
        autoSave();
    }

    void LifeScene::setOutAt(const std::string& zone)
    {
        const std::string activity = WalkActivity + zone;
        const RuleActivity* at = activityOf(activity);

        if (not at)
        {
            LOG_ERROR(DOM, "No walk to a zone '" << zone << "'");
            return;
        }

        // A life to try a zone with: of the age the walk asks, a grown youth's parts, and whatever
        // else its row asks of him, as its row asks it. A walk that eats and mends finds both in the bag
        save.age = static_cast<float>(std::max(intOf(at->fields, "fromAge"), TrialAge));

        // (a part with a most it can be is left where it is: the most is the rules' to move)
        for (const auto& part : save.parts)
        {
            if (caps.count(part) == 0)
                save.stats[part] += TrialParts;
        }

        for (const auto& r : at->requires)
        {
            const std::string stat = textOf(r, "stat");

            if (stat.empty() or stat == "age" or LifeSave::ofTheTown(stat))
                continue;

            save.stats[stat] = std::max(save.stats[stat], intOf(r, "needed"));
        }

        // What the world must know for it is known: the zone it asks studied, studied
        for (const auto& r : at->requires)
        {
            const std::string stat = textOf(r, "stat");
            const std::string known = "know.";

            if (stat.rfind(known, 0) == 0)
                save.knowledge[stat.substr(known.size())] = std::max(save.knowledge[stat.substr(known.size())], intOf(r, "needed"));
        }

        refreshHoldings();
        fillActivities();
        publish();

        beginWalk(activity);
    }

    void LifeScene::walkDo(const WalkAct& act)
    {
        if (not onWalk() or ended)
            return;

        RuleWalk answer;

        if (not askWalk("act", act, "", answer))
            return;

        walk = answer;
        save.walk = answer.state;

        takeStats(answer.after);
        writeEntries(answer.entries);

        if (textOf(walk.site, "screen") == "done")
        {
            endWalk();
            return;
        }

        // What he holds for the first time gets its row, under the screen
        refreshHoldings();

        for (const auto& r : save.holdEarned(holdings))
            addToLedger(r);

        showWalk();

        // What was asked and could not be done says why
        if (const std::string refused = textOf(walk.state.run, "refused"); not refused.empty())
            toast(refused);

        publishAll();
        autoSave();
    }

    void LifeScene::endWalk()
    {
        const std::string zone = textOf(walk.recipe, "zone");
        const std::string activity = WalkActivity + zone;
        const std::string name = textOf(walk.recipe, "name");
        const bool carried = intOf(walk.recipe, "downed") > 0;

        // The record of a walk made to its end is kept, in the place of the one he had of the
        // zone: a walk he was carried home from is not one to walk again
        if (not carried and not zone.empty())
        {
            WalkRecipe recipe;
            recipe.id = "r" + std::to_string(++save.recorded);
            recipe.fields = walk.recipe;
            recipe.steps = walk.state.record;
            recipe.marks = walk.state.marks;

            for (auto it = save.recipes.begin(); it != save.recipes.end();)
            {
                if (textOf(it->fields, "zone") == zone)
                {
                    if (save.queue.recipe == it->id)
                        save.queue.recipe = recipe.id;

                    it = save.recipes.erase(it);
                }
                else
                    ++it;
            }

            // The form of the Adventure page opens on the walk he has just made
            if (save.queue.left <= 0)
            {
                save.queue = WalkQueue{};
                save.queue.recipe = recipe.id;
            }

            save.recipes.push_back(std::move(recipe));
        }

        ++save.walksByHand;

        save.walk = WalkState{};
        walk = RuleWalk{};
        seekChosen.clear();
        workChosen.clear();

        closeWalk();

        // The walk is on the calendar from here: the way there, the stay and the way back run as
        // the months of any work do
        save.running = activity;
        save.monthsIn = 0;
        save.walking.clear();
        ++save.begun;
        rules.running = save.running;

        if (auto facts = ecsRef->getSystem<WorldFacts>())
            facts->setFact("activity." + activity + ".state", std::string("running"));

        paused = false;
        sinceMonth = 0.0f;

        refreshHoldings();

        for (const auto& r : save.holdEarned(holdings))
            addToLedger(r);

        fillActivities();
        publish();
        fillAdventure();

        if (carried)
            toast("Carried home from " + name);
        else
            toast("The record of " + name + " is kept: the Adventure page walks it again");

        autoSave();
    }

    // ---- the screens of a walk ------------------------------------------------------------------------

    EntityRef LifeScene::inWalk(const std::string& name) const
    {
        EntityRef root = walkPage;

        if (root.empty() or not root->has<Prefab>())
            return EntityRef{};

        return root->get<Prefab>()->findEntity(name);
    }

    void LifeScene::walkSay(const std::string& name, const std::string& text)
    {
        if (auto label = walkPiece<Label>(name); label and label->spec.text != text)
            label->setText(ecsRef, text);
    }

    void LifeScene::walkShow(const std::string& name, bool shown, const std::string& panel)
    {
        EntityRef node = inWalk(name);

        if (node.empty())
            return;

        // Out of its stack, or back in it: it is the wrap its layout holds that says so
        if (not panel.empty())
        {
            if (auto holder = walkPiece<Panel>(panel))
                wrapIn(holder->body, node)->get<PositionComponent>()->setVisibility(shown);
        }
        else
        {
            for (const char* column : {"walkLeft", "walkMiddle", "walkRight"})
            {
                if (EntityRef layout = inWalk(column); not layout.empty())
                    wrapIn(layout, node)->get<PositionComponent>()->setVisibility(shown);
            }
        }

        node->get<PositionComponent>()->setVisibility(shown);
    }

    void LifeScene::walkList(const std::string& name, const std::vector<ActivityGroup>& groups, const std::string& chosen)
    {
        auto list = walkPiece<ActivityList>(name);

        if (not list)
            return;

        const std::string signature = signatureOf(groups);

        if (walkFilled[name] != signature)
        {
            walkFilled[name] = signature;
            list->setRows(ecsRef, groups);
        }

        // Lit once the list holds its rows (execute): a row is not in its list until the next frame
        walkPicks[name] = chosen;
    }

    void LifeScene::walkLedger(const std::string& name, const std::vector<LedgerRowSpec>& rows)
    {
        auto ledger = walkPiece<ResourceLedger>(name);

        if (not ledger)
            return;

        const std::string signature = ledgerSignature(rows);

        if (walkFilled[name] == signature)
            return;

        walkFilled[name] = signature;

        ledger->clear(ecsRef);
        ledger->addGroup(ecsRef, name, "");   // Unlabelled: no heading

        for (const auto& row : rows)
            ledger->addRow(ecsRef, name, row, "");
    }

    void LifeScene::walkLog(const std::string& name, const std::vector<LogEntry>& lines)
    {
        auto log = walkPiece<EventLog>(name);

        if (not log)
            return;

        // What is there is kept while it still begins what is to be shown: the lines after it are
        // written under it. Else the whole of it again
        bool grown = log->size() <= lines.size();
        size_t at = 0;

        for (const auto& item : log->items)
        {
            if (not std::holds_alternative<EventLog::Row>(item))
                continue;

            const auto& row = std::get<EventLog::Row>(item);

            grown = grown and at < lines.size() and row.entry.text == lines[at].text and row.entry.figure == lines[at].figure;
            ++at;
        }

        if (not grown)
        {
            log->clear(ecsRef);
            at = 0;
        }

        for (; at < lines.size(); ++at)
            log->append(ecsRef, lines[at]);

        log->scrollToEnd(ecsRef);
    }

    bool LifeScene::buildWalk(const std::string& screen)
    {
        float width, height;
        windowSize(ecsRef, width, height);

        const bool narrow = width < NarrowBelow;
        const bool fighting = screen == "fight";
        const std::string& file = fighting ? (narrow ? opt.fightNarrowFile : opt.fightFile) : (narrow ? opt.siteNarrowFile : opt.siteFile);

        std::vector<std::string> errors;
        PrefabLoadOptions options;
        options.errors = &errors;

        auto spec = loadNodeSpec(ecsRef, file, options);

        if (not spec)
        {
            LOG_ERROR(DOM, "Could not load the walk's screen " << file);
            return false;
        }

        for (const auto& e : errors)
            LOG_ERROR(DOM, file << ": " << e);

        // The screen at the window's width, written over the file's before it is built: a panel
        // hands its width to what stands in it, so nothing else has to be told
        shapeWalk(*spec, fighting, narrow, width, height);

        EntityRef built = buildTree(ecsRef, *spec);

        if (built.empty() or not built->has<Prefab>())
        {
            LOG_ERROR(DOM, "Could not build the walk's screen " << file);
            return false;
        }

        closeWalk();

        walkPage = built;
        walkScreen = screen;
        walkNarrow = narrow;
        walkWidth = width;
        walkFresh = true;
        walkSettling = SettlePasses;

        // It leaves with the scene, like the page
        ecsRef->attach<SceneElement>(walkPage);

        // The veil takes the mouse: what is under it is neither hovered nor clicked
        if (EntityRef veil = inWalk("veil"); not veil.empty())
        {
            if (not veil->has<MouseEnterComponent>())
                ecsRef->attach<MouseEnterComponent>(veil, makeCallable<WalkNoOp>(WalkNoOp{}));

            if (not veil->has<MouseLeaveComponent>())
                ecsRef->attach<MouseLeaveComponent>(veil, makeCallable<WalkNoOp>(WalkNoOp{}));
        }

        // Its columns scroll under a finger, or a mouse held down, as the lists do
        for (const char* name : {"walkLeft", "walkMiddle", "walkRight"})
        {
            if (EntityRef column = inWalk(name); not column.empty() and column->has<VerticalLayout>())
                column->get<VerticalLayout>()->dragToScroll = true;
        }

        return true;
    }

    void LifeScene::closeWalk()
    {
        if (not walkPage.empty())
            ecsRef->removeEntity(walkPage);

        walkPage = EntityRef{};
        walkScreen.clear();
        walkFilled.clear();
        walkPicks.clear();
        autoLeft = 0.0f;
    }

    void LifeScene::showWalk()
    {
        if (not onWalk())
        {
            closeWalk();
            return;
        }

        // A walk opened from a save has no answer yet: the pages of the walk as it stands
        if (textOf(walk.site, "screen").empty())
        {
            RuleWalk answer;

            if (not askWalk("look", WalkAct{}, "", answer))
                return;

            walk = answer;
        }

        // A fight has the screen to itself; an event is answered on site, before its fight
        const std::string screen = textOf(walk.site, "screen") == "fight" ? "fight" : "site";

        if (walkScreen != screen and not buildWalk(screen))
            return;

        if (screen == "fight")
            fillFight();
        else
            fillSite();
    }

    void LifeScene::fitWalk(float width, float height)
    {
        if (walkPage.empty())
            return;

        // At another width the screen is built again from its file, the one of that width, and
        // filled again: what stands in a panel took its width when it was made
        if (std::abs(width - walkWidth) > 0.5f)
        {
            const std::string screen = walkScreen;

            if (buildWalk(screen))
            {
                if (screen == "fight")
                    fillFight();
                else
                    fillSite();
            }

            return;
        }

        // The veil is the window's size, from its corner: nothing of the page shows past it
        if (EntityRef veil = inWalk("veil"); not veil.empty())
        {
            auto pos = veil->get<PositionComponent>();

            pos->setX(0.0f);
            pos->setY(0.0f);
            pos->setWidth(width);
            pos->setHeight(height);
        }

        // The columns run to the bottom, and scroll what does not fit
        const bool fighting = walkScreen == "fight";
        const float tall = columnHeight(fighting, walkNarrow, height);

        for (const char* name : {"walkLeft", "walkMiddle", "walkRight"})
        {
            if (EntityRef column = inWalk(name); not column.empty())
                column->get<PositionComponent>()->setHeight(tall);
        }

        // In three columns a log fills its own to the bottom (the account, what is left of it
        // under what the choice would do); in one it keeps its height
        if (not walkNarrow)
        {
            if (auto log = walkPiece<EventLog>("record"))
                log->setHeight(ecsRef, logHeight(tall - TakenRoom));

            if (auto log = walkPiece<EventLog>("account"))
                log->setHeight(ecsRef, logHeight(tall - PreviewRoom));
        }
    }

    void LifeScene::fillSite()
    {
        EntitySystem* ecs = ecsRef;
        const ElementMap& site = walk.site;
        const bool asked = textOf(site, "screen") == "event";

        walkSay("kicker", textOf(site, "kicker"));
        walkSay("zoneTitle", textOf(site, "title"));
        walkSay("zoneLine", textOf(site, "line"));
        walkSay("daysLeft", textOf(site, "daysLeft"));
        walkSay("daysNote", textOf(site, "daysNote"));
        walkSay("headNote", textOf(site, "home"));

        // How he stands, at the head of the left column: his Vitality in the field and how well
        // the place is known, each over its rule, and what knowing it better would bring
        if (auto panel = walkPiece<Panel>("statePanel"); panel and panel->title and panel->title->spec.text != save.name)
            panel->setHeading(ecs, save.name);

        walkSay("vitality", textOf(site, "vitality"));
        walkSay("knowledge", textOf(site, "knowledge"));
        walkSay("knowledgeNote", textOf(site, "knowledgeNote"));

        for (auto [name, key] : {std::pair<const char*, const char*>{"vitalityBar", "vitalityPercent"}, {"knowledgeBar", "knowledgePercent"}})
        {
            if (auto bar = walkPiece<ProgressRule>(name))
                bar->setPercent(ecs, static_cast<float>(intOf(site, key)), not walkFresh);
        }

        // The strip of days: what is spent solid, what is left of the stay hatched after it
        if (auto strip = walkPiece<ProgressRule>("strip"))
        {
            const float spent = static_cast<float>(intOf(site, "percent"));

            // A screen built this pass is filled as it stands: what moves, moves from the next
            strip->setPercent(ecs, spent, not walkFresh);
            strip->setForecast(ecs, 100.0f);   // The hatch runs from the fill's head to where it is told: the end of the stay
            strip->setCaption(ecs, textOf(site, "caption"));
        }

        // The beasts he may seek: one that he does not know yet, or has not the days for, stands
        // there with what it asks and cannot be chosen
        std::vector<ActivityRowSpec> seeks;
        bool seekStands = false;
        std::string seekSaid;
        std::string seekCost;

        for (const auto& s : walk.seeks)
        {
            ActivityRowSpec row;
            row.id = textOf(s, "id");
            row.name = textOf(s, "name");
            row.glyph = "adventure";
            row.cost = textOf(s, "costText");

            // One line under the name: how rare it is and how well he knows it. A row that cannot
            // be chosen says why there
            row.until = textOf(s, "rank");

            // A beast he does not know has no cost to say yet
            if (row.cost.empty())
                row.cost = "?";

            if (intOf(s, "locked") > 0)
            {
                lockRow(row, textOf(s, "asks"));
                row.until = textOf(s, "rank") + " \xC2\xB7 " + textOf(s, "asks");
            }
            else if (intOf(s, "short") > 0 or asked)
            {
                lockRow(row, asked ? "SOMETHING WAITS FOR HIS ANSWER" : textOf(s, "asks"));

                if (not asked)
                {
                    row.until = textOf(s, "asks");
                    row.urgent = true;
                }
            }
            else if (row.id == seekChosen)
            {
                seekStands = true;
                seekSaid = textOf(s, "note");
                seekCost = row.cost;
            }

            seeks.push_back(row);
        }

        if (not seekStands)
            seekChosen.clear();

        walkList("seeks", seeks.empty() ? std::vector<ActivityGroup>{} : std::vector<ActivityGroup>{{"", seeks}}, seekChosen);

        // No beast lives there: the panel has nothing to say, and leaves the column
        walkShow("seekPanel", not walk.seeks.empty());

        if (auto seek = walkPiece<Button>("seek"))
        {
            seek->setCost(ecs, seekStands ? seekCost : std::string());
            seek->setDisabled(ecs, not seekStands);
        }

        if (auto observe = walkPiece<Button>("observe"))
        {
            observe->setCost(ecs, textOf(site, "watchCost"));
            observe->setDisabled(ecs, not seekStands);
        }

        // Under the list, in a room that is always there (a line that came and went would move
        // the buttons under the mouse): what is known of the beast chosen, or what to do
        walkSay("seekNote", seekStands ? seekSaid : "Choose a beast, then seek it or watch it from cover.");

        // The field work: gathering, the study of the place, a rest, a remedy
        std::vector<ActivityRowSpec> works;
        bool workStands = false;
        std::string workName;
        std::string workCost;

        for (const auto& w : walk.works)
        {
            ActivityRowSpec row;
            row.id = textOf(w, "id");
            row.name = textOf(w, "name");
            row.glyph = textOf(w, "tone") == "know" ? "study" : row.id == "rest" or row.id == "remedy" ? "vitality" : "work";
            row.until = textOf(w, "note");
            row.cost = textOf(w, "costText");

            if (intOf(w, "locked") > 0)
            {
                lockRow(row, textOf(w, "asks"));
                row.until = textOf(w, "asks");
                row.urgent = true;
            }
            else if (asked)
                lockRow(row, "SOMETHING WAITS FOR HIS ANSWER");
            else if (row.id == workChosen)
            {
                workStands = true;
                workName = row.name;
                workCost = row.cost;
            }

            works.push_back(row);
        }

        if (not workStands)
            workChosen.clear();

        walkList("works", {{"", works}}, workChosen);

        if (auto work = walkPiece<Button>("work"))
        {
            work->setLabel(ecs, workStands ? workName : "Do it");
            work->setCost(ecs, workStands ? workCost : std::string());
            work->setDisabled(ecs, not workStands);
        }

        walkSay("leaveNote", textOf(site, "leaveNote"));

        if (auto leave = walkPiece<Button>("leave"))
        {
            leave->setLabel(ecs, textOf(site, "leave"));
            leave->setDisabled(ecs, asked);
        }

        // An event waiting for his answer: its card stands over the record until he gives it
        walkShow("cardPanel", asked);

        if (asked)
        {
            walkSay("cardTitle", textOf(walk.card, "title"));
            walkSay("cardText", textOf(walk.card, "text"));
            walkSay("cardFoot", textOf(walk.card, "foot"));
        }

        for (int i = 0; i < MaxOptions; ++i)
        {
            const std::string button = "option" + std::to_string(i);
            const std::string line = "optionLine" + std::to_string(i);
            const bool there = asked and static_cast<size_t>(i) < walk.options.size();

            walkShow(button, there, "cardPanel");
            walkShow(line, there, "cardPanel");

            if (not there)
            {
                if (auto option = walkPiece<Button>(button))
                    option->setDisabled(ecs, true);

                continue;
            }

            const ElementMap& o = walk.options[i];
            const bool locked = intOf(o, "locked") > 0;

            if (auto option = walkPiece<Button>(button))
            {
                option->setLabel(ecs, textOf(o, "label"));
                option->setDisabled(ecs, locked);
            }

            walkSay(line, locked ? textOf(o, "line") + " \xC2\xB7 " + textOf(o, "asks") : textOf(o, "line"));
        }

        // The record so far: a line a step, numbered as the record numbers them
        std::vector<LogEntry> lines;

        for (const auto& step : walk.state.record)
        {
            LogEntry line;
            line.age = static_cast<float>(intOf(step, "n"));
            line.text = textOf(step, "text");
            line.figure = textOf(step, "figure");
            line.kind = textOf(step, "tone") == "loss" ? LogKind::Loss : LogKind::Coin;
            line.glyph = textOf(step, "glyph");

            lines.push_back(line);
        }

        walkLog("record", lines);

        if (auto panel = walkPiece<Panel>("recordPanel"); panel and panel->aside and panel->aside->spec.text != textOf(site, "written"))
            panel->setAside(ecs, textOf(site, "written"));

        walkSay("recordNote", textOf(site, "summary"));

        // What he carries, what he has taken, and what the record would cost to walk again
        for (auto [name, rows] : {std::pair<const char*, const RecordList*>{"bag", &walk.bag}, {"taken", &walk.taken}, {"home", &walk.home}})
        {
            std::vector<LedgerRowSpec> ledger;

            for (const auto& r : *rows)
            {
                LedgerRowSpec row;
                row.id = textOf(r, "id");
                row.name = textOf(r, "name");
                row.glyph = textOf(r, "glyph");
                row.value = textOf(r, "value");

                ledger.push_back(row);
            }

            walkLedger(name, ledger);
        }

        walkShow("takenPanel", not walk.taken.empty());

        walkSay("scribe", textOf(site, "scribe"));
    }

    void LifeScene::fillFight()
    {
        EntitySystem* ecs = ecsRef;
        const ElementMap& duel = walk.duel;
        const bool over = not textOf(duel, "over").empty();
        const bool handed = intOf(duel, "auto") > 0;

        walkSay("kicker", textOf(duel, "kicker"));
        walkSay("fightTitle", textOf(duel, "title"));
        walkSay("turn", textOf(duel, "turn"));
        walkSay("turnNote", textOf(duel, "cost") + " \xC2\xB7 " + textOf(duel, "daysLeft"));
        walkSay("quick", textOf(duel, "quick"));
        walkSay("warn", over ? std::string() : textOf(duel, "warn"));

        // The order of the turn: he in lapis where he stands, the beasts with what each will do
        for (int i = 0; i < MaxChips; ++i)
        {
            auto chip = walkPiece<Label>("chip" + std::to_string(i));

            if (not chip)
                continue;

            if (over or static_cast<size_t>(i) >= walk.order.size())
            {
                if (not chip->spec.text.empty())
                    chip->setText(ecs, "");

                continue;
            }

            const ElementMap& o = walk.order[i];
            const std::string tone = textOf(o, "tone");
            const std::string text = std::string(i > 0 ? "\xE2\x86\x92 " : "") + upper(textOf(o, "text")) + " \xC2\xB7 " + textOf(o, "note");
            const std::string color = tone == "now" ? "lapis" : tone == "foe" ? "ink" : "ink-muted";

            if (chip->spec.text != text)
                chip->setText(ecs, text);

            if (chip->spec.color != color)
                chip->setColor(ecs, color);
        }

        // He: his Vitality, his breath, what his stance makes of his blows and of his guard
        if (auto panel = walkPiece<Panel>("heroPanel"); panel and panel->title and panel->title->spec.text != save.name)
            panel->setHeading(ecs, save.name);

        const int vitality = intOf(duel, "vitality");
        const int most = std::max(1, intOf(duel, "vitalityMost"));

        walkSay("heroVit", "VITALITY " + std::to_string(vitality) + " OF " + std::to_string(most));

        if (auto bar = walkPiece<ProgressRule>("heroBar"))
            bar->setPercent(ecs, 100.0f * static_cast<float>(vitality) / static_cast<float>(most), not walkFresh);

        walkSay("low", textOf(duel, "low"));
        walkShow("low", not textOf(duel, "low").empty(), "heroPanel");
        walkSay("breath", textOf(duel, "breath"));

        if (auto bar = walkPiece<ProgressRule>("breathBar"))
            bar->setPercent(ecs, static_cast<float>(intOf(duel, "breathPercent")), not walkFresh);
        walkSay("stance", textOf(duel, "stance"));

        std::vector<LedgerRowSpec> hero;

        for (const auto& r : walk.hero)
        {
            LedgerRowSpec row;
            row.id = textOf(r, "id");
            row.name = textOf(r, "name");
            row.glyph = textOf(r, "glyph");
            row.value = textOf(r, "value");
            row.rate = textOf(r, "rate");

            hero.push_back(row);
        }

        walkLedger("hero", hero);

        // What he knows of what he fights: what he does not is there, greyed, with what it asks
        if (auto panel = walkPiece<Panel>("foePanel"); panel and panel->title and panel->title->spec.text != textOf(duel, "foeTitle"))
            panel->setHeading(ecs, textOf(duel, "foeTitle"));

        std::vector<LedgerRowSpec> foe;

        for (const auto& r : walk.foe)
        {
            LedgerRowSpec row;
            row.id = textOf(r, "id");
            row.name = textOf(r, "name");
            row.glyph = textOf(r, "glyph");
            row.value = textOf(r, "value");
            row.muted = intOf(r, "muted") > 0;

            foe.push_back(row);
        }

        walkLedger("foe", foe);

        // The beasts, a tile each: the one aimed at is the one chosen, one that fell is chosen no more
        std::vector<ActivityRowSpec> targets;
        std::string aimed;

        for (const auto& t : walk.targets)
        {
            ActivityRowSpec row;
            row.id = textOf(t, "id");
            row.name = textOf(t, "name");
            row.glyph = "adventure";
            row.cost = textOf(t, "value");
            row.costGlyph = "vitality";   // What stands beside its name is how it stands, not a time
            row.until = textOf(t, "note");
            row.urgent = intOf(t, "urgent") > 0;

            if (intOf(t, "dead") > 0 or over)
                row.state = ActivityState::Locked;
            else if (intOf(t, "chosen") > 0)
                aimed = row.id;

            targets.push_back(row);
        }

        walkList("targets", {{"", targets}}, aimed);

        // The menu of the turn, in its groups: what cannot be done now says why in the place of
        // what it would do
        std::vector<ActivityGroup> menu;
        std::string picked;

        for (const auto& m : walk.menu)
        {
            const std::string group = textOf(m, "group");

            if (menu.empty() or menu.back().label != group)
                menu.push_back({group, {}});

            ActivityRowSpec row;
            row.id = textOf(m, "id");
            row.name = textOf(m, "name");
            row.glyph = "swordsmanship";
            row.cost = textOf(m, "cost");
            row.until = textOf(m, "note");

            // A stance is taken at once and costs no turn: its tile says so by its ground
            row.months = group == "Stance" ? 0 : 1;

            if (intOf(m, "locked") > 0 or over or handed)
                row.state = ActivityState::Locked;
            else if (intOf(m, "chosen") > 0)
                picked = row.id;

            menu.back().rows.push_back(row);
        }

        walkList("menu", menu, picked);

        // What the choice looked at would do: the figures that are there to give, each on its line
        const ElementMap& preview = walk.preview;

        walkSay("previewTitle", over ? textOf(duel, "title") : textOf(preview, "title"));
        walkSay("previewText", over ? textOf(duel, "spoils") : textOf(preview, "text"));

        std::vector<LedgerRowSpec> figures;

        if (not over)
        {
            for (auto [key, label, glyph] : {std::tuple<const char*, const char*, const char*>{"against", "Against", "adventure"}, {"hits", "Hits", "swordsmanship"}, {"deals", "Deals", "strength"},
                                             {"costs", "Costs", "time"}, {"then", "Then", "quill"}, {"bites", "Comes back", "vitality"}, {"foeAfter", "It is left", "adventure"}})
            {
                const std::string value = textOf(preview, key);

                if (value.empty())
                    continue;

                LedgerRowSpec row;
                row.id = key;
                row.name = label;
                row.glyph = glyph;
                row.value = value;

                figures.push_back(row);
            }
        }

        walkLedger("preview", figures);

        const std::string refused = textOf(duel, "refused");

        walkSay("odds", over ? std::string() : refused.empty() ? textOf(preview, "odds") : refused);

        // One button does what is chosen; once the fight is over it is the way back to the woods
        if (auto act = walkPiece<Button>("act"))
        {
            const bool locked = intOf(preview, "locked") > 0;

            act->setLabel(ecs, over ? textOf(duel, "back") : textOf(preview, "button"));
            act->setDisabled(ecs, not over and (locked or handed), locked ? textOf(preview, "asks") : std::string());
        }

        if (auto hand = walkPiece<Button>("hand"))
        {
            hand->setLabel(ecs, textOf(duel, "hand"));
            hand->setDisabled(ecs, over);
        }

        walkShow("hand", not over, "previewPanel");

        // The account of the fight: a line a blow, the turn at its head, a tinted line a day gone
        std::vector<LogEntry> lines;

        for (const auto& a : walk.state.account)
        {
            LogEntry line;
            // A line between two turns (a day gone, the end) carries no turn
            line.age = textOf(a, "turn").empty() ? -1.0f : static_cast<float>(std::atoi(textOf(a, "turn").c_str()));
            line.text = textOf(a, "text");
            line.figure = textOf(a, "figure");
            line.kind = kindOf(textOf(a, "kind"));
            line.glyph = textOf(a, "kind") == "time" ? "time" : "";

            lines.push_back(line);
        }

        walkLog("account", lines);

        walkSay("footnote", textOf(duel, "footnote"));

        // A fight handed to his stance plays on by itself
        if (handed and not over and autoLeft <= 0.0f)
            autoLeft = opt.autoTurnMs;

        if (not handed or over)
            autoLeft = 0.0f;
    }

    void LifeScene::runWalk(float ms)
    {
        // A screen built a pass ago: its layouts hold what stands in them now. What was taken out
        // of a stack is taken out again (it is the wrap its layout holds that leaves, and the
        // layout did not hold it yet), and from here its rules may move
        if (walkSettling > 0 and not walkPage.empty())
        {
            // Not before its columns hold their panels: the layouts take what they are given when
            // their turn comes, which is not always the pass after
            bool held = true;

            for (const char* name : {"walkLeft", "walkMiddle", "walkRight"})
            {
                if (EntityRef column = inWalk(name); not column.empty() and column->has<VerticalLayout>())
                    held = held and not column->get<VerticalLayout>()->entities.empty();
            }

            if (held)
            {
                // And a pass or two more, for the stacks inside the panels
                --walkSettling;
                walkFresh = false;

                if (walkScreen == "fight")
                    fillFight();
                else
                    fillSite();
            }
        }

        // The rows chosen in a list filled a moment ago are lit once the list holds them: a list
        // of the walk's screen, or the records' on the page
        {
            for (auto it = walkPicks.begin(); it != walkPicks.end();)
            {
                auto list = walkPiece<ActivityList>(it->first);

                if (not list)
                    list = piece<ActivityList>(it->first);

                if (not list or list->selected() == it->second)
                    it = walkPicks.erase(it);
                else if (it->second.empty() or list->find(ecsRef, it->second))
                {
                    const std::string id = it->second;

                    it = walkPicks.erase(it);
                    list->select(ecsRef, id);
                }
                else
                    ++it;
            }
        }

        if (autoLeft <= 0.0f or not onWalk() or walkScreen != "fight")
            return;

        autoLeft -= ms;

        if (autoLeft > 0.0f)
            return;

        autoLeft = 0.0f;

        walkDo(WalkAct{"step", "", 0});
    }

    // ---- what he presses there ------------------------------------------------------------------------

    void LifeScene::onWalkSelect(const ActivitySelectedEvent& event)
    {
        if (event.list == RecordsList)
        {
            if (event.id != save.queue.recipe)
                chooseRecord(event.id);

            return;
        }

        if (not onWalk() or ended)
            return;

        if (event.list == SeeksList and event.id != seekChosen)
        {
            seekChosen = event.id;
            fillSite();
        }
        else if (event.list == WorksList and event.id != workChosen)
        {
            workChosen = event.id;
            fillSite();
        }
        else if (event.list == MenuList and not event.id.empty() and event.id != textOf(walk.preview, "id"))
        {
            walkDo(WalkAct{"pick", event.id, 0});
        }
        else if (event.list == TargetsList and not event.id.empty())
        {
            for (const auto& t : walk.targets)
            {
                if (textOf(t, "id") == event.id and intOf(t, "chosen") <= 0)
                    walkDo(WalkAct{"target", "", intOf(t, "n")});
            }
        }
    }

    void LifeScene::onWalkConfirm(const ActivityActivatedEvent& event)
    {
        if (event.list == RecordsList)
        {
            if (event.id != save.queue.recipe)
                chooseRecord(event.id);

            queueWalks(1);

            return;
        }

        if (not onWalk() or ended)
            return;

        // A second click on a row does what its button does
        if (event.list == SeeksList)
        {
            seekChosen = event.id;
            onWalkButton(SeekTag);
        }
        else if (event.list == WorksList)
        {
            workChosen = event.id;
            onWalkButton(WorkTag);
        }
        else if (event.list == MenuList)
        {
            if (event.id != textOf(walk.preview, "id"))
                walkDo(WalkAct{"pick", event.id, 0});

            onWalkButton(ActTag);
        }
    }

    bool LifeScene::onWalkButton(const std::string& tag)
    {
        if (tag.rfind(WalkTag, 0) != 0)
            return false;

        // The Adventure page's own: the form of a queue
        if (tag == FewerTag or tag == MoreTag)
        {
            save.queue.count = std::clamp(save.queue.count + (tag == MoreTag ? 1 : -1), 1, MostWalks);
            fillAdventure();
            autoSave();
        }
        else if (tag == StopTag)
        {
            // The next of the ways it may stop, round and round
            const RecordList& stops = shelf.stops;

            for (size_t i = 0; i < stops.size(); ++i)
            {
                if (textOf(stops[i], "id") != textOf(shelf.form, "stop"))
                    continue;

                const ElementMap& next = stops[(i + 1) % stops.size()];

                save.queue.stop = textOf(next, "id");
                save.queue.value = intOf(next, "value");
                save.queue.item = textOf(next, "item");

                break;
            }

            fillAdventure();
            autoSave();
        }
        else if (tag == OnceTag)
            queueWalks(1);
        else if (tag == ManyTag)
            queueWalks(std::max(1, save.queue.count));
        else if (tag == LastTag)
        {
            save.lastWalk = not save.lastWalk;
            fillAdventure();
            autoSave();
        }

        if (not onWalk() or ended)
            return true;

        if (tag == SeekTag and not seekChosen.empty())
        {
            for (const auto& s : walk.seeks)
            {
                if (textOf(s, "id") == seekChosen)
                {
                    const WalkAct act{"seek", textOf(s, "mob"), intOf(s, "count")};

                    seekChosen.clear();
                    walkDo(act);

                    break;
                }
            }
        }
        else if (tag == ObserveTag and not seekChosen.empty())
        {
            for (const auto& s : walk.seeks)
            {
                if (textOf(s, "id") == seekChosen)
                {
                    walkDo(WalkAct{"observe", textOf(s, "mob"), 0});

                    break;
                }
            }
        }
        else if (tag == WorkTag and not workChosen.empty())
        {
            // "gather.berries" is a gathering of berries; the others are their own name
            const std::string gather = "gather.";

            if (workChosen.rfind(gather, 0) == 0)
                walkDo(WalkAct{"gather", workChosen.substr(gather.size()), 0});
            else
                walkDo(WalkAct{workChosen, "", 0});
        }
        else if (tag == LeaveTag)
            walkDo(WalkAct{"leave", "", 0});
        else if (tag.rfind(OptionTag, 0) == 0)
        {
            const size_t index = static_cast<size_t>(std::atoi(tag.substr(std::string(OptionTag).size()).c_str()));

            if (index < walk.options.size())
                walkDo(WalkAct{"option", textOf(walk.options[index], "id"), 0});
        }
        else if (tag == ActTag)
            walkDo(WalkAct{textOf(walk.duel, "over").empty() ? "do" : "back", "", 0});
        else if (tag == HandTag)
            walkDo(WalkAct{"auto", "", 0});

        return true;
    }

    // ---- the Adventure page: the records kept, and a queue of walks --------------------------------

    int LifeScene::adventureTab() const
    {
        auto tabs = piece<Tabs>("tabs");

        if (not tabs)
            return -1;

        for (size_t i = 0; i < tabs->spec.items.size(); ++i)
        {
            const std::string label = lower(tabs->spec.items[i].label);

            if (label == AdventureName or label == "walks")
                return static_cast<int>(i);
        }

        return -1;
    }

    bool LifeScene::adventureKnown() const
    {
        if (not save.recipes.empty())
            return true;

        // A walk has its row, open to him or not yet: there is something to say of it
        for (const auto& a : activities)
        {
            if (not textOf(a.fields, "zone").empty() and boolOf(a.fields, "listed"))
                return true;
        }

        return false;
    }

    void LifeScene::showAdventure(bool shown)
    {
        adventureShown = shown and adventureKnown();

        // The three pages of the middle column take each other's place: showTown lays them
        showTown(adventureShown ? false : townShown);

        if (adventureShown)
            fillAdventure();
    }

    void LifeScene::fitAdventure(float width, float height)
    {
        EntitySystem* ecs = ecsRef;

        if (EntityRef body = named(AdventureName); not body.empty())
        {
            body->get<PositionComponent>()->setWidth(width);
            body->get<PositionComponent>()->setHeight(height);
        }

        if (auto list = piece<ActivityList>(RecordsName))
            list->setSize(ecs, width, 0.0f);

        for (const char* name : {"recordsHead", "recordsNote", "queueHead", "queueTitle", "queueTake", "queueUse", "queueBring", "queueTeach", "queueWarn", "atWalk"})
        {
            if (auto label = piece<Label>(name))
                label->setWidth(ecs, width);
        }

        for (const char* name : {"queueCount", "queueGo"})
        {
            if (EntityRef row = named(name); not row.empty())
                row->get<PositionComponent>()->setWidth(width);
        }
    }

    void LifeScene::chooseRecord(const std::string& id)
    {
        // A queue under way keeps its record: the form is its own until it is done
        if (save.queue.left > 0 or not save.walking.empty())
            return;

        if (save.queue.recipe == id)
            return;

        save.queue = WalkQueue{};
        save.queue.recipe = id;

        fillAdventure();
    }

    void LifeScene::markAdventure()
    {
        // Its tab is on the page for who has somewhere to walk to, and for no one else
        if (auto tabs = piece<Tabs>("tabs"); tabs and adventureTab() >= 0 and tabs->tabs[adventureTab()].shown != adventureKnown())
            tabs->setShown(ecsRef, adventureTab(), adventureKnown());
    }

    void LifeScene::fillAdventure(bool again)
    {
        EntitySystem* ecs = ecsRef;

        // A page just built has a list with nothing in it, whatever the last one held
        if (again)
            walkFilled.erase(RecordsName);

        markAdventure();

        if (named(AdventureName).empty())
            return;

        if (not askWalk("records", WalkAct{}, "", shelf))
            return;

        auto registry = ecs->getSystem<GlossRegistry>();

        // The records kept, then the zones open to him that he has no record of
        std::vector<ActivityRowSpec> rows;
        std::string lit;
        std::string signature;

        for (const auto& r : shelf.shelf)
        {
            ActivityRowSpec row;
            row.id = textOf(r, "id");
            row.name = textOf(r, "title");
            row.glyph = "adventure";
            row.rank = textOf(r, "kind");
            row.cost = textOf(r, "time");
            row.glossKey = RecordGloss + row.id;

            const bool kept = textOf(r, "state") == "kept";

            // What is said of it stands on a line of its own; a row that cannot be chosen says why
            // in its place
            if (intOf(r, "locked") > 0)
                lockRow(row, kept ? textOf(r, "asks") : textOf(r, "note"));
            else
                row.until = textOf(r, "note");

            if (row.state != ActivityState::Locked and row.id == save.queue.recipe)
                lit = row.id;

            signature += row.id + "|" + row.until + "|" + row.cost + "|" + std::to_string(static_cast<int>(row.state)) + ";";

            rows.push_back(row);

            if (not registry)
                continue;

            // The gloss: what the walk takes and what it brings home, as the record saw it
            GlossSpec gloss;
            gloss.inlineValues = true;
            gloss.title = row.name;
            gloss.aside = row.rank;
            gloss.text = kept ? textOf(r, "note") : textOf(r, "brings");

            if (kept)
            {
                gloss.rows.push_back({"IT TAKES", "", "", true});
                gloss.rows.push_back({"Time", textOf(r, "time"), "time"});
                gloss.rows.push_back({"Vitality", textOf(r, "vitality"), intOf(r, "locked") > 0 ? "muted" : "loss"});
                gloss.rows.push_back({"From the bag", textOf(r, "takes"), "loss"});
                gloss.rows.push_back({"IT BRINGS", "", "", true});

                if (not textOf(r, "brings").empty())
                    gloss.rows.push_back({textOf(r, "brings"), "", "gain"});

                if (not textOf(r, "learns").empty())
                    gloss.rows.push_back({textOf(r, "learns"), "", "muted"});

                gloss.footnote = "WALKED AGAIN WITH NO FIGHT FOUGHT";
            }
            else
                gloss.footnote = "THE FIRST WALK IS MADE BY HAND";

            registry->set(RecordGloss + row.id, gloss);
        }

        if (auto list = piece<ActivityList>(RecordsName))
        {
            if (walkFilled[RecordsName] != signature or list->spec.groups.empty() != rows.empty())
            {
                walkFilled[RecordsName] = signature;
                list->setRows(ecs, rows.empty() ? std::vector<ActivityGroup>{} : std::vector<ActivityGroup>{{"", rows}});
            }

            walkPicks[RecordsName] = lit;
        }

        auto say = [this, ecs](const char* name, const std::string& text) {
            if (auto label = piece<Label>(name); label and label->spec.text != text)
                label->setText(ecs, text);
        };

        // The form: the record chosen, how many walks, what they take and bring. A zone with no
        // record has one thing to offer: the first walk, made by hand
        const ElementMap& form = shelf.form;
        const std::string chosen = save.queue.recipe;
        const bool unwalked = chosen.rfind(ZoneRow, 0) == 0;
        const bool kept = not textOf(form, "recipe").empty();
        const bool busy = save.queue.left > 0 or not save.walking.empty();
        const std::string blocked = textOf(form, "blocked");
        const bool free = save.running.empty() and not onWalk() and not ended;

        say("queueTitle", kept ? upper(textOf(form, "title")) + " \xC2\xB7 " + textOf(form, "total")
                         : unwalked ? "NO RECORD OF IT YET: THE FIRST WALK IS MADE BY HAND" : "CHOOSE A RECORD ABOVE");
        say("queueTake", kept ? "Takes: " + textOf(form, "take") : std::string());
        say("queueUse", kept ? "Uses: " + textOf(form, "use") : std::string());
        say("queueBring", kept ? "Brings: " + textOf(form, "bring") : std::string());
        say("queueTeach", kept ? "Teaches: " + textOf(form, "teach") : std::string());
        say("queueWarn", kept ? (blocked.empty() ? textOf(form, "warning") : blocked) : std::string());

        if (auto stop = piece<Button>("queueStop"))
        {
            stop->setLabel(ecs, kept ? "Stop: " + lower(textOf(form, "stopLabel")) : "Stop: never");
            stop->setDisabled(ecs, not kept or busy);
        }

        for (const char* name : {"queueFewer", "queueMore"})
        {
            if (auto button = piece<Button>(name))
                button->setDisabled(ecs, not kept or busy);
        }

        if (auto once = piece<Button>("walkOnce"))
        {
            once->setLabel(ecs, unwalked ? "Walk it by hand" : "Walk once");
            once->setDisabled(ecs, busy or not free or (not kept and not unwalked) or (kept and not blocked.empty()));
        }

        if (auto many = piece<Button>("walkMany"))
        {
            many->setLabel(ecs, kept ? textOf(form, "many") : "Queue");
            many->setCost(ecs, kept ? std::to_string(intOf(form, "months")) + " mo" : std::string());
            many->setDisabled(ecs, busy or not free or not kept or not blocked.empty());
        }

        // A queue under way: which walk of how many, and the button that ends it with this one
        std::string walking;

        if (busy)
        {
            const int total = std::max(1, save.queue.total);
            const int at = std::clamp(total - save.queue.left, 1, total);

            walking = "AT WORK NOW \xC2\xB7 WALK " + std::to_string(at) + " OF " + std::to_string(total) + " \xC2\xB7 NO FIGHT IS FOUGHT AGAIN";

            if (save.lastWalk)
                walking += " \xC2\xB7 THE LAST: THE QUEUE STOPS WITH IT";
        }

        say("atWalk", walking);

        if (auto last = piece<Button>("queueLast"))
        {
            last->setLabel(ecs, save.lastWalk ? "Go on with the queue" : "Finish this walk, then stop");
            last->setDisabled(ecs, not busy or save.queue.left <= 0);
        }
    }

    void LifeScene::queueWalks(int count)
    {
        if (ended or onWalk())
            return;

        if (not save.running.empty())
        {
            appendLog({save.age, "Already at work: " + activityName(save.running), LogKind::Note, "", ""});
            return;
        }

        // A zone with no record: the first walk is made by hand, as its row of the Life page begins it
        if (save.queue.recipe.rfind(ZoneRow, 0) == 0)
        {
            const std::string activity = WalkActivity + save.queue.recipe.substr(ZoneRow.size());

            showAdventure(false);
            onConfirm(ActivityActivatedEvent{ActivitiesList, activity});

            return;
        }

        if (save.queue.recipe.empty())
            return;

        // The stop as the form shows it: what a queue counts is the rules' to say
        save.queue.stop = textOf(shelf.form, "stop").empty() ? std::string("never") : textOf(shelf.form, "stop");
        save.queue.value = intOf(shelf.form, "stopValue");
        save.queue.item = textOf(shelf.form, "stopItem");
        save.queue.left = std::max(1, count);
        save.queue.total = save.queue.left;
        save.lastWalk = false;

        if (beginReplay())
            ++save.begun;

        publish();
        fillAdventure();
        autoSave();
    }

    void LifeScene::stopQueue(const std::string& why, bool loss)
    {
        save.queue.left = 0;
        save.queue.total = 0;
        save.lastWalk = false;
        save.walking.clear();

        if (why.empty())
            return;

        // Said on the Life page: a queue that stops is a work that has to be chosen again
        appendLog({save.age, why, loss ? LogKind::Loss : LogKind::Note, "", "adventure"});
        toast(why);
    }

    bool LifeScene::beginReplay()
    {
        const WalkRecipe* recipe = nullptr;

        for (const auto& r : save.recipes)
        {
            if (r.id == save.queue.recipe)
                recipe = &r;
        }

        if (not recipe)
        {
            stopQueue("", false);
            return false;
        }

        const std::string activity = WalkActivity + textOf(recipe->fields, "zone");
        const RuleActivity* at = activityOf(activity);

        // The way there may have shut since the record was made: an age, a thing it asks
        if (not at or boolOf(at->fields, "locked") or not boolOf(at->fields, "listed"))
        {
            stopQueue("The queue stopped: the way to " + textOf(recipe->fields, "name") + " is shut to him now", true);
            return false;
        }

        RuleWalk answer;

        if (not askWalk("begin", WalkAct{}, recipe->id, answer))
        {
            stopQueue("", false);
            return false;
        }

        if (not answer.stopped.empty())
        {
            stopQueue(answer.stopped, false);
            return false;
        }

        if (not answer.blocked.empty())
        {
            // A queue told to end when the bag runs short ends there; another says what it lacks
            const bool asked = save.queue.stop == "bag";

            stopQueue(asked ? "The bag is short of " + answer.blocked + ": the queue is done" : "The queue stopped at the door: the bag lacks " + answer.blocked, not asked);
            return false;
        }

        // What the record takes from the bag is taken as he sets out
        takeStats(answer.after);

        --save.queue.left;
        save.walking = recipe->id;

        save.running = activity;
        save.monthsIn = 0;
        rules.running = save.running;

        if (auto facts = ecsRef->getSystem<WorldFacts>())
            facts->setFact("activity." + activity + ".state", std::string("running"));

        paused = false;
        sinceMonth = 0.0f;

        return true;
    }

    void LifeScene::homeFromWalk()
    {
        // A walk made by hand gave what it gave on site: its months were the way and the stay
        if (save.walking.empty())
        {
            fillAdventure();
            return;
        }

        // By the record: its days are folded into him now that he is home
        RuleWalk answer;

        if (askWalk("term", WalkAct{}, save.walking, answer))
        {
            takeStats(answer.after);
            writeEntries(answer.entries);
            ++save.walksByRecord;
        }

        const bool more = save.queue.left > 0 and not save.lastWalk;

        save.walking.clear();

        refreshHoldings();

        // What the walk left him too hurt to go on with ends the queue with the life
        if (not death.empty())
            return;

        if (more)
        {
            // The list as he stands now: the next walk asks what the first did
            rules.done = save.terms();
            rules.activities(save.age, save.character(), activities);

            beginReplay();
        }
        else
        {
            const bool queued = save.queue.total > 1;

            save.queue.left = 0;
            save.queue.total = 0;
            save.lastWalk = false;

            if (queued)
                toast("The queue is done");
        }

        fillAdventure();
    }
}
