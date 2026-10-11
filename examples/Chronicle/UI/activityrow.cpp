#include "activityrow.h"

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL.h>
#elif __linux__
#include <SDL2/SDL.h>
#elif _WIN32
#include <SDL.h>
#endif

#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <cstdlib>
#include <string>

#include "logger.h"

#include "ECS/callable.h"
#include "2D/position.h"
#include "2D/simple2dobject.h"
#include "2D/decoratedshapes.h"
#include "UI/prefab.h"
#include "UI/sizer.h"
#include "UI/focusable.h"
#include "UI/themesystem.h"

#include "Core/textmetrics.h"
#include "Core/offstage.h"
#include "gloss.h"

using namespace pg;

// The row and the list also live on their root entity as plain components. They are rebuilt
// from their spec, never saved, so the archive form is empty.
namespace pg
{
    template <>
    void serialize(Archive& archive, const chronicle::ActivityRow&)
    {
        archive.startSerialization("ActivityRow");
        archive.endSerialization();
    }

    template <>
    chronicle::ActivityRow deserialize(const UnserializedObject&)
    {
        return chronicle::ActivityRow{};
    }

    template <>
    void serialize(Archive& archive, const chronicle::ActivityList&)
    {
        archive.startSerialization("ActivityList");
        archive.endSerialization();
    }

    template <>
    chronicle::ActivityList deserialize(const UnserializedObject&)
    {
        return chronicle::ActivityList{};
    }
}

namespace chronicle
{
    namespace
    {
        constexpr const char * const DOM = "Chronicle.Activity";

        // No-op event so the root's hover components carry a valid callable; the system reacts
        // to HoverChangedEvent, not to these.
        struct ActivityNoOp {};

        constexpr float Pad = 12.0f;             // space-3 padding
        constexpr float MarkColumn = 26.0f;
        constexpr float Gap = 12.0f;             // Between the columns
        constexpr float TitleHeight = 24.0f;     // The tab line box
        constexpr float MiddleGap = 4.0f;        // space-1 under the title line
        constexpr float RankGap = 8.0f;
        constexpr float EdgeWidth = 3.0f;        // border-frame
        constexpr float RingInset = 2.0f;
        constexpr float CostMarkGap = 4.0f;
        constexpr float EachGap = 2.0f;
        constexpr float UntilGap = 2.0f;         // Above the line saying when it closes
        constexpr float TilePad = 8.0f;          // space-2 padding of a tile
        constexpr float TileLineGap = 2.0f;      // Between a tile's name and its time
        constexpr float TileGap = 8.0f;          // space-2 between two tiles, on a line and from line to line
        constexpr int TileNameLines = 2;         // A tile's name wraps to that many lines, then elides
        constexpr float ActionWidth = 56.0f;     // The room of a table line's button, at its right end
        constexpr float CellGap = 12.0f;         // Between two cells of a table line
        constexpr float InsetPadX = 5.0f;        // A tile's time in its box: beside it,
        constexpr float InsetPadY = 2.0f;        // over and under it,
        constexpr float InsetGap = 6.0f;         // and between the box and the name
        constexpr float MajorFrameGap = 2.0f;    // Between the two rules of a major tile's frame
        constexpr float MaxRequirementWidth = 300.0f;
        constexpr float DoubleReleaseMs = 400.0f;
        constexpr int HoverFrames = 12;          // After a list's rows are built again: the frames in which the hover is worked out again
        constexpr float HeadingHeight = 24.0f;
        constexpr float HeadingAbove = 12.0f;    // space-3, none for the first group
        constexpr float HeadingBelow = 4.0f;     // space-1
        constexpr float ScrollWidth = 4.0f;      // The scroll thumb
        constexpr float ScrollStep = 34.0f;      // Half an idle row per wheel notch

        const std::string NoBreakSpace = "\xC2\xA0";
        const std::string Minus = "\xE2\x88\x92";   // U+2212

        // An activity that takes no time is done at once
        std::string monthsText(int months)
        {
            if (months <= 0)
                return "NOW";

            return std::to_string(months) + " mo";
        }

        // What a row writes for its time. A tile done at once writes nothing there: its ground
        // already says it is a thing done at once
        std::string costText(const ActivityRowSpec& spec)
        {
            if (not spec.cost.empty())
                return spec.cost;

            if (spec.tile and spec.months <= 0)
                return "";

            return monthsText(spec.months);
        }

        // The months he still has to begin a thing that closes; its last month says so
        std::string closesText(const ActivityRowSpec& spec)
        {
            if (spec.closes < 0)
                return "";

            return spec.closes == 0 ? std::string("last mo") : std::to_string(spec.closes) + " mo";
        }

        // "RANK 2 \xC2\xB7 DONE 3": the rank, then how often it was done, on one label
        std::string rankText(const ActivityRowSpec& spec)
        {
            if (spec.rank.empty() or spec.count.empty())
                return spec.rank + spec.count;

            return spec.rank + " \xC2\xB7 " + spec.count;
        }

        // EntityRef::has looks an entity created during the frame up in the pool and dereferences
        // the miss. The arrow falls back to the pointer the ref holds, so go through it.
        template <typename Comp>
        bool hasComp(EntityRef entity)
        {
            return not entity.empty() and entity->template has<Comp>();
        }

        // A part added to a row that is already clipped (a list that scrolls) is clipped like
        // it: the prefab only hands its clip down when it moves or resizes.
        void clipLike(EntitySystem* ecs, EntityRef root, EntityRef part)
        {
            if (not hasComp<ClippedTo>(root) or hasComp<ClippedTo>(part))
                return;

            ecs->attach<ClippedTo>(part, root->get<ClippedTo>()->clipperId);
        }

        EntityRef entityOf(EntitySystem* ecs, _unique_id id)
        {
            if (id == 0)
                return EntityRef{};

            return ecs->getEntity(id);
        }

        void setElement(EntityRef entity, const std::string& element)
        {
            if (entity.empty() or not entity->has<ThemeComponent>())
                return;

            auto theme = entity->get<ThemeComponent>();

            if (theme->element != element)
                theme->setElement(element);
        }

        void setVisible(EntityRef entity, bool visible)
        {
            if (entity.empty())
                return;

            auto pos = entity->get<PositionComponent>();

            if (pos->isVisible() != visible)
                pos->setVisible(visible);
        }

        // A label painted by an activity element instead of its label.<style>.<color> key. The
        // style must be the element's font: the label measures itself with it.
        Label makeActivityText(EntitySystem* ecs, const std::string& style, const std::string& element, const std::string& text, int z, Overflow overflow = Overflow::Grow, float width = 0.0f)
        {
            LabelSpec ls;
            ls.style = style;
            ls.text = text;
            ls.z = z;
            ls.overflow = overflow;
            ls.width = width;

            Label label = makeLabel(ecs, ls);
            label.entity->get<ThemeComponent>()->setElement(element);

            // A row is often made while the game runs (the list filled again, the work begun)
            offstage(label.entity);

            return label;
        }

        float textWidth(EntitySystem* ecs, const std::string& style, const std::string& text)
        {
            const TextStyle& s = ecs->getSystem<ThemeSystem>()->style(style);

            return ecs->getSystem<TTFTextSystem>()->measureText(s.fontAlias, text, 1.0f, 0.0f, 0.0f, s.letterSpacingPx).width;
        }

        void placeIn(EntityRef entity, _unique_id rootId, float x, float y, float dz)
        {
            auto anchor = entity->get<UiAnchor>();

            anchor->setLeftAnchor(PosAnchor{rootId, AnchorType::Left});
            anchor->setLeftMargin(x);
            anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
            anchor->setTopMargin(y);
            anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, dz});
        }

        void removeChild(EntitySystem* ecs, EntityRef root, _unique_id child)
        {
            if (child == 0)
                return;

            root->get<Prefab>()->childrenIds.erase(child);
            ecs->removeEntity(child);
        }

        // The copy of the row that lives on its root, or the caller's when there is none.
        ActivityRow* liveRow(ActivityRow* self)
        {
            if (hasComp<ActivityRow>(self->root))
                return self->root.get<ActivityRow>().component;

            return self;
        }

        // Runs a setter on the live row and brings the caller's copy in step.
        template <typename Fn>
        void onLive(ActivityRow* self, Fn fn)
        {
            ActivityRow* live = liveRow(self);

            fn(*live);

            if (live != self)
                *self = *live;
        }

        std::string groundElement(const ActivityRowState& st)
        {
            // A compact row's ground says its kind: work that takes months, a thing done at once,
            // what he cannot do yet, the work he is at
            if (st.compact)
            {
                if (st.state == ActivityState::Running)
                    return "activity.kind.running";

                if (st.state == ActivityState::Locked)
                    return "activity.kind.locked";

                const std::string kind = st.featured ? "activity.kind.featured" : st.major ? "activity.kind.major" : st.adventure ? "activity.kind.adventure"
                                       : st.instant ? "activity.kind.instant" : "activity.kind.timed";

                return st.hovered ? kind + ".hover" : kind;
            }

            if (st.state == ActivityState::Running)
                return "activity.row.ground.running";

            // The line of a table that is the one chosen is tinted whole, with its edge
            if (st.table and st.selected)
                return "activity.row.ground.selected";

            if (st.state == ActivityState::Idle and st.hovered)
                return "activity.row.ground.hover";

            return st.stripe ? "activity.row.ground.stripe" : "activity.row.ground";
        }

        std::string edgeElement(const ActivityRowState& st)
        {
            // A compact row's ground already says it is the work he is at: its edge is the
            // selection's alone, so the two are never taken for one another
            if (st.state == ActivityState::Running and not st.compact)
                return "activity.row.edge.running";

            return st.selected ? "activity.row.edge.selected" : "activity.row.edge";
        }

        std::string markElement(const ActivityRowState& st)
        {
            // The mark in a major tile's corner is gold while the tile can be taken
            if (st.major and st.state != ActivityState::Running)
                return st.state == ActivityState::Locked ? "activity.major.mark.locked" : "activity.major.mark";

            // And the one of a tile that takes him out of the town, in its own colour
            if (st.adventure and st.tile and st.state != ActivityState::Running)
                return st.state == ActivityState::Locked ? "activity.adventure.mark.locked" : "activity.adventure.mark";

            switch (st.state)
            {
            case ActivityState::Running:
                return "activity.mark.running";

            case ActivityState::Locked:
                return "activity.mark.locked";

            case ActivityState::Idle:
            default:
                return "activity.mark";
            }
        }

        std::string frameElement(const ActivityRowState& st)
        {
            const std::string frame = st.major ? "activity.major.frame" : "activity.adventure.frame";

            return st.state == ActivityState::Locked ? frame + ".locked" : frame;
        }

        // A tile's name is set in the control face, a row's in the display one
        std::string nameElement(const ActivityRowState& st)
        {
            const std::string name = st.tile or st.table ? "activity.tile.name" : "activity.name";

            return st.state == ActivityState::Locked ? name + ".locked" : name;
        }

        // State and flags -> the element of every part. Also the rule of the last row and the ring.
        void paint(EntitySystem* ecs, EntityRef root)
        {
            if (root.empty() or not root->has<ActivityRowState>())
                return;

            const ActivityRowState& st = *root->get<ActivityRowState>().component;
            const bool locked = st.state == ActivityState::Locked;

            setElement(entityOf(ecs, st.ground), groundElement(st));
            setElement(entityOf(ecs, st.edge), edgeElement(st));
            setElement(entityOf(ecs, st.frame), frameElement(st));
            setElement(entityOf(ecs, st.mark), markElement(st));
            setElement(entityOf(ecs, st.name), nameElement(st));
            setElement(entityOf(ecs, st.cost), locked ? "activity.cost.locked" : "activity.cost");

            // No time, no time mark: a tile done at once leaves that line empty
            setVisible(entityOf(ecs, st.costMark), not (st.tile and st.instant));
            setVisible(entityOf(ecs, st.rule), not st.last and not st.tile);
            setVisible(entityOf(ecs, st.ring), st.keyboardFocus and not locked);
        }

        // The same paint through the row's own refs. A row built during a frame is not in the
        // entity pool yet, so a lookup by id finds nothing and the paint would be lost.
        void paintRow(const ActivityRow& row)
        {
            if (not hasComp<ActivityRowState>(row.root))
                return;

            const ActivityRowState& st = *row.root.get<ActivityRowState>().component;
            const bool locked = st.state == ActivityState::Locked;

            setElement(row.ground, groundElement(st));
            setElement(row.edge, edgeElement(st));
            setElement(row.frame, frameElement(st));
            setElement(row.mark.entity, markElement(st));
            setElement(row.name.entity, nameElement(st));
            setElement(row.cost.entity, locked ? "activity.cost.locked" : "activity.cost");

            // A tile's time stands in its box, when it has one to say; what closes it at the right of the line under
            const bool timed = not row.cost.spec.text.empty();

            setVisible(row.costMark.entity, not st.table and not (st.tile and (st.instant or not timed)));
            setVisible(row.inset, st.tile and timed);
            setVisible(row.insetFrame, st.tile and timed);
            setElement(row.inset, locked ? "activity.tile.time.locked" : "activity.tile.time");

            if (row.closing)
            {
                const bool closes = row.spec.closes >= 0;

                setVisible(row.closing->entity, closes);
                setVisible(row.closeMark.entity, closes);
                setElement(row.closing->entity, locked ? "activity.closes.locked" : row.spec.urgent ? "activity.closes.urgent" : "activity.closes");
                setElement(row.closeMark.entity, locked ? "activity.closes.mark.locked" : row.spec.urgent ? "activity.closes.mark.urgent" : "activity.closes.mark");
            }

            setVisible(row.rule, not st.last and not st.tile);
            setVisible(row.ring, st.keyboardFocus and not locked);
        }

        // The entity a layout slot holds for a child: the builder wraps a node in a Prefab whose
        // main entity is what the kind produced.
        EntityRef mainOf(EntityRef entity)
        {
            if (entity->has<Prefab>())
            {
                auto prefab = entity->get<Prefab>();

                if (prefab->namedChildrenIds.count("MainEntity") > 0)
                    return prefab->getEntity("MainEntity");
            }

            return entity;
        }

        BaseLayout* layoutOf(EntityRef entity)
        {
            if (entity->has<VerticalLayout>())
                return entity->get<VerticalLayout>().component;

            if (entity->has<HorizontalLayout>())
                return entity->get<HorizontalLayout>().component;

            return nullptr;
        }

        // A line of tiles: a prefab the tiles stand on side by side, so the list's clip and its
        // scroll reach them through it. The list's body stacks the lines.
        EntityRef makeTileLine(EntitySystem* ecs, float width, int z)
        {
            auto line = makeAnchoredPrefab(ecs, Offstage, Offstage, static_cast<float>(z));
            line.get<PositionComponent>()->setWidth(width);
            ecs->attach<ActivityLineState>(line.entity);

            return line.entity;
        }

        void placeTile(EntityRef line, EntityRef tile, float x)
        {
            auto anchor = tile->get<UiAnchor>();

            anchor->setLeftAnchor(PosAnchor{line.id, AnchorType::Left});
            anchor->setLeftMargin(x);
            anchor->setTopAnchor(PosAnchor{line.id, AnchorType::Top});
            anchor->setZConstrain(PosConstrain{line.id, AnchorType::Z});

            line->get<Prefab>()->addToPrefab(tile);
            line->get<ActivityLineState>()->tiles.push_back(tile);
        }

        // The tiles of a line are as tall as the tallest asks to be, and the line holds them with
        // the gap to the next. Nothing moves when they already are.
        void levelLine(EntitySystem* ecs, EntityRef line)
        {
            float tallest = 0.0f;

            for (auto& tile : line->get<ActivityLineState>()->tiles)
            {
                if (hasComp<ActivityRow>(tile))
                    tallest = std::max(tallest, tile->get<ActivityRow>()->natural);
            }

            for (auto& tile : line->get<ActivityLineState>()->tiles)
            {
                if (hasComp<ActivityRow>(tile))
                    tile->get<ActivityRow>()->setMinHeight(ecs, tallest);
            }

            auto pos = line->get<PositionComponent>();

            if (std::abs(pos->height - (tallest + TileGap)) > 0.01f)
                pos->setHeight(tallest + TileGap);
        }

        // Collects the refs the layouts hold, not ids: the rows of a list built during a frame
        // are not in the entity pool yet.
        void walkList(EntityRef entity, std::vector<EntityRef>& rows, std::vector<EntityRef>& headings)
        {
            if (entity.empty())
                return;

            if (entity->has<ActivityRowState>())
            {
                rows.push_back(entity);
                return;
            }

            if (entity->has<ActivityHeadingState>())
            {
                headings.push_back(entity);
                return;
            }

            // A line of tiles: its rows, left to right
            if (entity->has<ActivityLineState>())
            {
                for (auto& tile : entity->get<ActivityLineState>()->tiles)
                    walkList(tile, rows, headings);

                return;
            }

            if (auto layout = layoutOf(entity))
            {
                for (auto& child : layout->entities)
                    walkList(child, rows, headings);

                return;
            }

            EntityRef main = mainOf(entity);

            if (main.id != entity.id)
                walkList(main, rows, headings);
        }
    }

    void registerActivityComponents(EntitySystem* ecs)
    {
        auto registry = ecs->getComponentRegistry();

        if (not registry->hasTypeId<ActivityRow>())
            ecs->registerFlagComponent<ActivityRow>();

        if (not registry->hasTypeId<ActivityList>())
            ecs->registerFlagComponent<ActivityList>();
    }

    std::string gainsText(const std::vector<Gain>& gains)
    {
        std::string text;

        for (const auto& gain : gains)
        {
            if (not text.empty())
                text += "   ";

            text += gain.stat + NoBreakSpace + (gain.amount < 0 ? Minus : "+") + std::to_string(std::abs(gain.amount));
        }

        return text;
    }

    ActivityRow makeActivityRow(EntitySystem* ecs, const ActivityRowSpec& specIn)
    {
        ActivityRowSpec spec = specIn;
        spec.months = std::max(0, spec.months);
        spec.percent = std::clamp(spec.percent, 0.0f, 100.0f);
        spec.compact = spec.compact or spec.tile;   // A tile is a compact row, stacked

        if (not ecs->getSystem<ActivitySystem>())
        {
            LOG_ERROR(DOM, "makeActivityRow needs an ActivitySystem in this ECS");
            return ActivityRow{};
        }

        if (spec.id.empty())
            LOG_ERROR(DOM, "ActivityRow '" << spec.name << "' has no id; its events will carry an empty one");

        const float W = spec.width;
        const int z = spec.z;

        ActivityRow row;
        row.spec = spec;

        // Born off the stage, like every part of it: the list places it a frame later, and until
        // then nothing of it shows where it was made
        auto root = makeAnchoredPrefab(ecs, Offstage, Offstage, static_cast<float>(z));
        root.get<PositionComponent>()->setWidth(W);
        row.root = root.entity;

        const _unique_id rootId = root.id;
        auto prefab = root.get<Prefab>();

        auto state = ecs->attach<ActivityRowState>(root.entity);
        state->id = spec.id;
        state->state = spec.state;
        state->stripe = spec.stripe;
        state->compact = spec.compact;
        state->instant = spec.months <= 0;
        state->featured = spec.featured;
        state->major = spec.major and spec.tile;
        state->tile = spec.tile;
        state->adventure = spec.adventure and spec.tile;
        state->table = not spec.tile and not spec.compact and spec.glyph.empty();

        ecs->attach<FocusableComponent>(root.entity);

        if (auto focusOrder = ecs->getSystem<FocusOrderSystem>())
        {
            focusOrder->add(rootId);
            focusOrder->setEnabled(rootId, spec.state != ActivityState::Locked);
        }

        // Ground, rule, edge and ring follow the root's size through their anchors
        auto ground = makeUiSimple2DShape(ecs, Shape2D::Square, W, 1.0f);
        offstage(ground.entity);
        ground.get<UiAnchor>()->fillIn(root.get<UiAnchor>());
        ground.get<UiAnchor>()->setZConstrain(PosConstrain{rootId, AnchorType::Z});
        ecs->attach<ThemeComponent>(ground.entity, "activity.row.ground");
        prefab->addToPrefab(ground.entity);
        row.ground = ground.entity;
        state->ground = ground.entity.id;

        auto rule = makeUiSimple2DShape(ecs, Shape2D::Square, W, 1.0f);
        offstage(rule.entity);
        {
            auto anchor = rule.get<UiAnchor>();

            anchor->setLeftAnchor(PosAnchor{rootId, AnchorType::Left});
            anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
            anchor->setBottomAnchor(PosAnchor{rootId, AnchorType::Bottom});
            anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 1.0f});
        }
        rule.get<PositionComponent>()->setHeight(1.0f);
        ecs->attach<ThemeComponent>(rule.entity, "activity.row.rule");
        prefab->addToPrefab(rule.entity);
        row.rule = rule.entity;
        state->rule = rule.entity.id;

        auto edge = makeUiSimple2DShape(ecs, Shape2D::Square, EdgeWidth, 1.0f);
        offstage(edge.entity);
        {
            auto anchor = edge.get<UiAnchor>();

            anchor->setLeftAnchor(PosAnchor{rootId, AnchorType::Left});
            anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
            anchor->setBottomAnchor(PosAnchor{rootId, AnchorType::Bottom});
            anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 2.0f});
        }
        edge.get<PositionComponent>()->setWidth(EdgeWidth);
        ecs->attach<ThemeComponent>(edge.entity, "activity.row.edge");
        prefab->addToPrefab(edge.entity);
        row.edge = edge.entity;
        state->edge = edge.entity.id;

        // The focus ring sits inside the row, so rows stacked edge to edge never overlap it
        auto ring = makeStrokeRect2DShape(ecs, 1.0f, 1.0f, {255.0f, 255.0f, 255.0f, 255.0f}, ecs->getSystem<ThemeSystem>()->border("border-rule"));
        offstage(ring.entity);
        {
            auto anchor = ring.get<UiAnchor>();

            anchor->setLeftAnchor(PosAnchor{rootId, AnchorType::Left});
            anchor->setLeftMargin(RingInset);
            anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
            anchor->setRightMargin(RingInset);
            anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
            anchor->setTopMargin(RingInset);
            anchor->setBottomAnchor(PosAnchor{rootId, AnchorType::Bottom});
            anchor->setBottomMargin(RingInset);
            anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 2.0f});
        }
        ring.get<PositionComponent>()->setVisible(false);
        ecs->attach<ThemeComponent>(ring.entity, "activity.row.ring");
        prefab->addToPrefab(ring.entity);
        row.ring = ring.entity;
        state->ring = ring.entity.id;

        // A major tile is framed like an illuminated leaf, two gold rules around it: what turns a
        // life is not one more tile of the grid. One that takes him out of the town has a frame
        // of its own, a single rule
        if (spec.tile and (spec.major or spec.adventure))
        {
            auto frame = spec.major ? makeStrokeRect2DShape(ecs, 1.0f, 1.0f, {255.0f, 255.0f, 255.0f, 255.0f}, 1.0f, MajorFrameGap, true)
                                    : makeStrokeRect2DShape(ecs, 1.0f, 1.0f, {255.0f, 255.0f, 255.0f, 255.0f}, 1.0f);
            offstage(frame.entity);
            frame.get<UiAnchor>()->fillIn(root.get<UiAnchor>());
            frame.get<UiAnchor>()->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 1.0f});
            ecs->attach<ThemeComponent>(frame.entity, spec.major ? "activity.major.frame" : "activity.adventure.frame");
            prefab->addToPrefab(frame.entity);
            row.frame = frame.entity;
            state->frame = frame.entity.id;

            // And it shows its mark, at the left of the line under its name: its time has the
            // corner (resize sets how far down)
            row.mark = makeMark(ecs, {spec.glyph, MarkSize::S24, "ink-muted", z + 3});
            offstage(row.mark.entity);
            {
                auto anchor = row.mark.entity->get<UiAnchor>();

                anchor->setLeftAnchor(PosAnchor{rootId, AnchorType::Left});
                anchor->setLeftMargin(TilePad);
                anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
                anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 3.0f});
            }
            row.mark.entity->get<ThemeComponent>()->setElement(spec.major ? "activity.major.mark" : "activity.adventure.mark");
            prefab->addToPrefab(row.mark.entity);
            state->mark = row.mark.entity.id;
        }

        // A tile's time stands in a box in its top right corner, where a tile that takes time
        // is told at a glance from one that takes none: the box is sized to what it says (fitInset)
        if (spec.tile)
        {
            auto inset = makeUiSimple2DShape(ecs, Shape2D::Square, 1.0f, 1.0f);
            offstage(inset.entity);
            {
                auto anchor = inset.get<UiAnchor>();

                anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
                anchor->setRightMargin(TilePad);
                anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
                anchor->setTopMargin(TilePad);
                anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 1.0f});
            }
            ecs->attach<ThemeComponent>(inset.entity, "activity.tile.time");
            prefab->addToPrefab(inset.entity);
            row.inset = inset.entity;

            auto insetFrame = makeStrokeRect2DShape(ecs, 1.0f, 1.0f, {255.0f, 255.0f, 255.0f, 255.0f}, 1.0f);
            offstage(insetFrame.entity);
            insetFrame.get<UiAnchor>()->fillIn(inset.get<UiAnchor>());
            insetFrame.get<UiAnchor>()->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 2.0f});
            ecs->attach<ThemeComponent>(insetFrame.entity, "activity.tile.time.frame");
            prefab->addToPrefab(insetFrame.entity);
            row.insetFrame = insetFrame.entity;
        }

        // Mark: S24 (the design system draws 22; the kit registers 18 and 24), centred in its column.
        // A tile has none, but a major one: its width is the name's
        if (not spec.tile and not spec.glyph.empty())
        {
            row.mark = makeMark(ecs, {spec.glyph, MarkSize::S24, "ink-muted", z + 3});
            offstage(row.mark.entity);
            {
                auto anchor = row.mark.entity->get<UiAnchor>();

                anchor->setLeftAnchor(PosAnchor{rootId, AnchorType::Left});
                anchor->setLeftMargin(Pad + (MarkColumn - px(MarkSize::S24)) / 2.0f);
                anchor->setVerticalCenter(PosAnchor{rootId, AnchorType::VerticalCenter});
                anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 3.0f});
            }
            row.mark.entity->get<ThemeComponent>()->setElement("activity.mark");
            prefab->addToPrefab(row.mark.entity);
            state->mark = row.mark.entity.id;
        }

        // The cost and its time mark. On a row: the right column, on the title line's baseline, `each`
        // under it. On a tile: in the box of its top right corner
        row.cost = makeActivityText(ecs, "control", "activity.cost", costText(spec), z + 4);
        row.costMark = makeMark(ecs, {spec.costGlyph.empty() ? std::string("time") : spec.costGlyph, MarkSize::S14, "status-time", z + 3});
        offstage(row.costMark.entity);

        {
            auto anchor = row.cost.entity->get<UiAnchor>();
            auto markAnchor = row.costMark.entity->get<UiAnchor>();

            anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
            anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 4.0f});

            markAnchor->setVerticalCenter(PosAnchor{row.cost.entity.id, AnchorType::VerticalCenter});
            markAnchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 3.0f});

            if (spec.tile)
            {
                anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
                anchor->setRightMargin(TilePad + InsetPadX);
                anchor->setTopMargin(TilePad + InsetPadY);

                markAnchor->setRightAnchor(PosAnchor{row.cost.entity.id, AnchorType::Left});
                markAnchor->setRightMargin(CostMarkGap);
            }
            else
            {
                anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
                anchor->setRightMargin(Pad + row.rightInset());
                anchor->setTopMargin(Pad + baselineShift(ecs, "tab", "control"));

                markAnchor->setRightAnchor(PosAnchor{row.cost.entity.id, AnchorType::Left});
                markAnchor->setRightMargin(CostMarkGap);
            }
        }

        prefab->addToPrefab(row.cost.entity);
        state->cost = row.cost.entity.id;

        row.costMark.entity->get<ThemeComponent>()->setElement("activity.cost.mark");
        prefab->addToPrefab(row.costMark.entity);
        state->costMark = row.costMark.entity.id;

        // A compact row keeps the name, the time and the closing: the rest is its gloss's
        if (not spec.each.empty() and not spec.compact)
        {
            row.each = makeActivityText(ecs, spec.eachTone.empty() ? "caption" : "body-sm", spec.eachTone.empty() ? std::string("activity.each") : "activity.each." + spec.eachTone, spec.each, z + 4);

            auto anchor = row.each->entity->get<UiAnchor>();

            anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
            anchor->setRightMargin(Pad + row.rightInset());

            // Under the cost; on a line of a table, level with what is said under the name
            if (state->table)
            {
                anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
                anchor->setTopMargin(Pad + TitleHeight + MiddleGap);
            }
            else
            {
                anchor->setTopAnchor(PosAnchor{row.cost.entity.id, AnchorType::Bottom});
                anchor->setTopMargin(EachGap);
            }

            anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 4.0f});
            prefab->addToPrefab(row.each->entity);
        }

        // Title line: the name, then the rank on its baseline. A tile's name is set smaller, at the
        // tile's left edge, and wraps in its width
        if (spec.tile)
        {
            LabelSpec ls;
            ls.style = "control";
            ls.text = spec.name;
            ls.z = z + 4;
            ls.overflow = Overflow::Wrap;
            ls.width = row.tileNameWidth();
            ls.maxLines = TileNameLines;

            row.name = makeLabel(ecs, ls);
            offstage(row.name.entity);
            row.name.entity->get<ThemeComponent>()->setElement("activity.tile.name");
            placeIn(row.name.entity, rootId, TilePad, TilePad, 4.0f);

            // What closes it: a stopwatch and the months he still has to begin it, at the right of
            // the line under the name, there or not as the spec says (paintRow)
            row.closing = makeActivityText(ecs, "control", "activity.closes", closesText(spec), z + 4);
            {
                auto anchor = row.closing->entity->get<UiAnchor>();

                anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
                anchor->setRightMargin(TilePad);
                anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
                anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 4.0f});
            }
            prefab->addToPrefab(row.closing->entity);

            row.closeMark = makeMark(ecs, {"chrono", MarkSize::S14, "status-time", z + 3});
            offstage(row.closeMark.entity);
            {
                auto anchor = row.closeMark.entity->get<UiAnchor>();

                anchor->setRightAnchor(PosAnchor{row.closing->entity.id, AnchorType::Left});
                anchor->setRightMargin(CostMarkGap);
                anchor->setVerticalCenter(PosAnchor{row.closing->entity.id, AnchorType::VerticalCenter});
                anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 3.0f});
            }
            row.closeMark.entity->get<ThemeComponent>()->setElement("activity.closes.mark");
            prefab->addToPrefab(row.closeMark.entity);
        }
        else
        {
            row.name = state->table ? makeActivityText(ecs, "control", "activity.tile.name", spec.name, z + 4)
                                    : makeActivityText(ecs, "tab", "activity.name", spec.name, z + 4);
            placeIn(row.name.entity, rootId, row.bodyLeft(), Pad, 4.0f);
        }

        prefab->addToPrefab(row.name.entity);
        state->name = row.name.entity.id;

        if ((not spec.rank.empty() or not spec.count.empty()) and not spec.compact)
        {
            row.rank = makeActivityText(ecs, "caption", "activity.rank", rankText(spec), z + 4);

            auto anchor = row.rank->entity->get<UiAnchor>();

            anchor->setLeftAnchor(PosAnchor{row.name.entity.id, AnchorType::Right});
            anchor->setLeftMargin(RankGap);
            anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
            anchor->setTopMargin(Pad + baselineShift(ecs, "tab", "caption"));
            anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 4.0f});
            prefab->addToPrefab(row.rank->entity);
        }

        // A line of a table: a cell of two lines past the cost, and a small button at the row's end
        if (not spec.compact and not (spec.asideTop.empty() and spec.asideBottom.empty()))
        {
            const float right = Pad + (spec.action.empty() ? 0.0f : ActionWidth + CellGap);

            auto cell = [&](const std::string& text, const std::string& tone, const char* style, const char* line, float top) {
                Label label = makeActivityText(ecs, style, std::string("activity.cell.") + line + (tone.empty() ? "" : "." + tone), text, z + 4, Overflow::Ellipsis, spec.asideWidth);
                auto anchor = label.entity->get<UiAnchor>();

                anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
                anchor->setRightMargin(right);
                anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
                anchor->setTopMargin(top);
                anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 4.0f});
                prefab->addToPrefab(label.entity);

                return label;
            };

            row.asideTop = cell(spec.asideTop, spec.asideTopTone, "control", "top", Pad + baselineShift(ecs, "tab", "control"));
            row.asideBottom = cell(spec.asideBottom, spec.asideBottomTone, "body-sm", "bottom", Pad + TitleHeight + MiddleGap);
        }

        if (not spec.compact and not spec.action.empty())
        {
            ButtonSpec bs;
            bs.variant = ButtonVariant::Quiet;
            bs.label = spec.action;
            bs.tag = spec.actionTag;
            bs.z = z + 4;

            row.action = makeButton(ecs, bs);

            auto anchor = row.action->root->get<UiAnchor>();

            anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
            anchor->setRightMargin(Pad);
            anchor->setVerticalCenter(PosAnchor{rootId, AnchorType::VerticalCenter});
            prefab->addToPrefab(row.action->root);

            // What cannot be chosen cannot be walked either
            if (spec.state == ActivityState::Locked)
                row.action->setDisabled(ecs, true);
        }

        row.fitName(ecs);

        row.buildMiddle(ecs);
        row.buildUntil(ecs);
        row.resize(ecs);

        if (not spec.glossKey.empty())
            attachGloss(ecs, row.root, spec.glossKey);

        paintRow(row);

        // The row lives on its root, where a list finds it
        registerActivityComponents(ecs);
        ecs->attachGeneric<ActivityRow>(row.root, row);

        // The glide's tween holds the rule by address: started on the live row, never on this copy
        if (spec.state == ActivityState::Running and spec.glideMs > 0.0f)
            row.setGlide(ecs, spec.glideTo, spec.glideMs);

        return row;
    }

    float ActivityRow::middleWidth() const
    {
        const bool table = not spec.tile and not spec.compact and spec.glyph.empty();
        const float costWidth = (table ? 0.0f : px(MarkSize::S14) + CostMarkGap) + cost.entity.get<PositionComponent>()->width;
        const float eachWidth = each ? each->entity.get<PositionComponent>()->width : 0.0f;
        const float rightWidth = std::max(costWidth, eachWidth);

        return spec.width - bodyLeft() - rightWidth - rightInset() - Gap - Pad;
    }

    float ActivityRow::bodyLeft() const
    {
        return spec.glyph.empty() and not spec.tile ? Pad : Pad + MarkColumn + Gap;
    }

    float ActivityRow::rightInset() const
    {
        if (spec.compact)
            return 0.0f;

        return (spec.action.empty() ? 0.0f : ActionWidth + CellGap)
             + (spec.asideTop.empty() and spec.asideBottom.empty() ? 0.0f : spec.asideWidth + CellGap);
    }

    void ActivityRow::buildMiddle(EntitySystem* ecs)
    {
        // A compact row has no middle block, in any state: its gloss says what this would
        if (spec.compact)
            return;

        const float x = bodyLeft();
        const float y = Pad + TitleHeight + MiddleGap;
        const float middle = middleWidth();
        const int z = spec.z;
        auto prefab = root->get<Prefab>();

        // A line of a table says one plain thing under its name, whatever its state but running:
        // what the place gives, or what it asks of him while it is shut
        if (not spec.line.empty() and spec.state != ActivityState::Running)
        {
            gains = makeActivityText(ecs, "body-sm", spec.state == ActivityState::Locked ? "activity.line.locked" : "activity.line", spec.line, z + 4, Overflow::Ellipsis, middle);
            placeIn(gains->entity, root.id, x, y, 4.0f);
            prefab->addToPrefab(gains->entity);
            clipLike(ecs, root, gains->entity);

            return;
        }

        switch (spec.state)
        {
        case ActivityState::Running:
        {
            ProgressRuleSpec ps;
            ps.width = middle;
            ps.percent = spec.percent;
            ps.nib = true;
            ps.caption = spec.caption;
            ps.z = z + 3;

            progress = makeProgressRule(ecs, ps);
            placeIn(progress->root, root.id, x, y, 3.0f);
            prefab->addToPrefab(progress->root);
            clipLike(ecs, root, progress->root);
            break;
        }

        case ActivityState::Locked:
        {
            RequirementListSpec rs;
            rs.width = std::min(MaxRequirementWidth, middle);
            rs.dense = true;
            rs.items = spec.requirements;
            rs.z = z + 3;

            reqs = makeRequirementList(ecs, rs);
            placeIn(reqs->root, root.id, x, y, 3.0f);
            prefab->addToPrefab(reqs->root);
            clipLike(ecs, root, reqs->root);
            break;
        }

        case ActivityState::Idle:
        default:
            // What it brings wraps in the room between the mark and the cost: a gain keeps its
            // figure (a no-break space joins them), a long list takes a second line
            gains = makeActivityText(ecs, "tick", "activity.gains", gainsText(spec.gains), z + 4, Overflow::Wrap, middle);
            placeIn(gains->entity, root.id, x, y, 4.0f);
            prefab->addToPrefab(gains->entity);
            clipLike(ecs, root, gains->entity);
            break;
        }
    }

    void ActivityRow::clearMiddle(EntitySystem* ecs)
    {
        if (gains)
            removeChild(ecs, root, gains->entity.id);

        if (progress)
            removeChild(ecs, root, progress->root.id);

        if (reqs)
            removeChild(ecs, root, reqs->root.id);

        gains.reset();
        progress.reset();
        reqs.reset();
    }

    void ActivityRow::buildUntil(EntitySystem* ecs)
    {
        if (spec.until.empty())
        {
            if (until)
                removeChild(ecs, root, until->entity.id);

            until.reset();

            return;
        }

        if (until)
        {
            until->setText(ecs, spec.until);
        }
        else
        {
            // On a tile it wraps in the tile's width: a date is longer than a tile may be wide
            if (spec.tile)
                until = makeActivityText(ecs, "tick", "activity.until", spec.until, spec.z + 4, Overflow::Wrap, std::max(1.0f, spec.width - 2.0f * TilePad));
            else
                until = makeActivityText(ecs, "tick", "activity.until", spec.until, spec.z + 4);

            placeIn(until->entity, root.id, spec.tile ? TilePad : bodyLeft(), 0.0f, 4.0f);
            root->get<Prefab>()->addToPrefab(until->entity);
            clipLike(ecs, root, until->entity);
        }

        setElement(until->entity, spec.urgent ? "activity.until.urgent" : "activity.until");
    }

    void ActivityRow::resize(EntitySystem* ecs)
    {
        float middle = 0.0f;

        if (progress)
            middle = progress->root.get<PositionComponent>()->height;
        else if (reqs)
            middle = reqs->height(ecs);
        else if (gains)
            middle = std::max(static_cast<float>(gains->lineHeightPx), gains->entity.get<PositionComponent>()->height);   // An empty gains line keeps its height

        // The title line, then the middle block; a compact row has the title line alone, and a
        // tile its name, on as many lines as it takes, over its time
        float content = Pad + TitleHeight;
        float pad = Pad;

        if (spec.tile)
        {
            fitInset();

            // The name, beside the box of its time; under them a line for what closes it, at its
            // right, and for the mark of a tile that shows one, at its left
            const float nameHeight = std::max(static_cast<float>(name.lineHeightPx), name.entity.get<PositionComponent>()->height);
            const float boxHeight = inset.empty() ? 0.0f : inset->get<PositionComponent>()->height;
            const float lineTop = TilePad + std::max(nameHeight, boxHeight) + TileLineGap;

            // The line is kept when it says nothing: every tile of the grid is as tall as the others
            float line = static_cast<float>(cost.lineHeightPx);

            if (closing)
                closing->entity->get<UiAnchor>()->setTopMargin(lineTop);

            if (not mark.entity.empty())
            {
                mark.entity->get<UiAnchor>()->setTopMargin(lineTop);
                line = std::max(line, px(MarkSize::S24));
            }

            content = lineTop + line;
            pad = TilePad;
        }
        else if (not spec.compact)
        {
            content += MiddleGap + middle;
        }

        float height = content + pad;

        // The closing line stands under what the row holds, whatever that is, and the row holds it
        if (until)
        {
            until->entity->get<UiAnchor>()->setTopMargin(content + UntilGap);

            height += UntilGap + std::max(static_cast<float>(until->lineHeightPx), until->entity.get<PositionComponent>()->height);
        }

        // What it asks for itself, then what the list asks of it: the tiles of a line are level
        natural = height;
        height = std::max(height, spec.minHeight);

        auto pos = root.get<PositionComponent>();

        if (std::abs(pos->height - height) > 0.01f)
        {
            pos->setHeight(height);

            // The list re-stacks on the height change; tell anyone who follows the prefab.
            ecs->sendEvent(PrefabChangedEvent{root.id});
        }
    }

    float ActivityRow::tileNameWidth() const
    {
        // The tile's width, less the box its time stands in when it has one to say
        const float box = cost.spec.text.empty() ? 0.0f
                        : 2.0f * InsetPadX + px(MarkSize::S14) + CostMarkGap + cost.entity.get<PositionComponent>()->width + InsetGap;

        return std::max(1.0f, spec.width - 2.0f * TilePad - box);
    }

    void ActivityRow::fitInset()
    {
        if (inset.empty())
            return;

        const float width = 2.0f * InsetPadX + px(MarkSize::S14) + CostMarkGap + cost.entity.get<PositionComponent>()->width;
        const float height = 2.0f * InsetPadY + std::max(static_cast<float>(cost.lineHeightPx), px(MarkSize::S14));

        auto pos = inset->get<PositionComponent>();

        if (std::abs(pos->width - width) > 0.01f)
            pos->setWidth(width);

        if (std::abs(pos->height - height) > 0.01f)
            pos->setHeight(height);
    }

    void ActivityRow::fitName(EntitySystem* ecs)
    {
        // A tile's name has the tile's width, and wraps in it; its closing too
        if (spec.tile)
        {
            fitInset();
            name.setWidth(ecs, tileNameWidth());

            if (until)
                until->setWidth(ecs, std::max(1.0f, spec.width - 2.0f * TilePad));

            return;
        }

        // The name hugs its text, so the rank follows it; it is elided only when the room
        // between the mark and the cost is too small for it.
        const float rankWidth = rank ? rank->entity.get<PositionComponent>()->width + RankGap : 0.0f;
        const float room = std::max(0.0f, middleWidth() - rankWidth);
        const bool elide = textWidth(ecs, name.spec.style, name.spec.text) > room;
        const Overflow overflow = elide ? Overflow::Ellipsis : Overflow::Grow;

        if (name.spec.overflow != overflow)
        {
            name.spec.overflow = overflow;
            name.entity->get<TTFText>()->setOverflow(overflow);
        }

        // For a name that grows this only re-measures it.
        name.setWidth(ecs, room);
    }

    void ActivityRow::setState(EntitySystem* ecs, ActivityState newState)
    {
        onLive(this, [ecs, newState](ActivityRow& r) {
            if (r.spec.state == newState)
                return;

            r.spec.state = newState;

            auto st = r.root.get<ActivityRowState>();
            st->state = newState;

            // A running or a locked row cannot stay selected.
            if (st->selected and newState != ActivityState::Idle)
            {
                if (auto system = ecs->getSystem<ActivitySystem>(); system and st->list != 0)
                    system->select(ecs->getEntity(st->list), "");
                else
                    st->selected = false;
            }

            if (auto focusOrder = ecs->getSystem<FocusOrderSystem>())
                focusOrder->setEnabled(r.root.id, newState != ActivityState::Locked);

            r.clearMiddle(ecs);
            r.buildMiddle(ecs);
            r.resize(ecs);

            paintRow(r);
        });
    }

    void ActivityRow::setPercent(EntitySystem* ecs, float percent, bool animate)
    {
        onLive(this, [ecs, percent, animate](ActivityRow& r) {
            r.spec.percent = std::clamp(percent, 0.0f, 100.0f);
            r.spec.glideMs = 0.0f;

            if (r.progress)
                r.progress->setPercent(ecs, r.spec.percent, animate);
        });
    }

    void ActivityRow::setGlide(EntitySystem* ecs, float percent, float ms)
    {
        onLive(this, [ecs, percent, ms](ActivityRow& r) {
            r.spec.glideTo = std::clamp(percent, 0.0f, 100.0f);
            r.spec.glideMs = std::max(0.0f, ms);

            if (r.progress)
                r.progress->setGlide(ecs, r.spec.glideTo, r.spec.glideMs);
        });
    }

    void ActivityRow::setCaption(EntitySystem* ecs, const std::string& text)
    {
        onLive(this, [ecs, &text](ActivityRow& r) {
            r.spec.caption = text;

            if (r.progress)
            {
                r.progress->setCaption(ecs, text);
                r.resize(ecs);
            }
        });
    }

    void ActivityRow::setGains(EntitySystem* ecs, const std::vector<Gain>& newGains)
    {
        onLive(this, [ecs, &newGains](ActivityRow& r) {
            r.spec.gains = newGains;

            if (r.gains)
            {
                r.gains->setText(ecs, gainsText(newGains));
                r.resize(ecs);
            }
        });
    }

    void ActivityRow::setRequirements(EntitySystem* ecs, const std::vector<Requirement>& items)
    {
        onLive(this, [ecs, &items](ActivityRow& r) {
            r.spec.requirements = items;

            if (r.reqs)
            {
                r.reqs->setItems(ecs, items);
                r.resize(ecs);
            }
        });
    }

    void ActivityRow::setRequirement(EntitySystem* ecs, size_t index, int current, int needed)
    {
        onLive(this, [ecs, index, current, needed](ActivityRow& r) {
            if (index >= r.spec.requirements.size())
            {
                LOG_ERROR(DOM, "setRequirement(" << index << ") out of range (" << r.spec.requirements.size() << " requirements)");
                return;
            }

            r.spec.requirements[index].current = current;
            r.spec.requirements[index].needed = needed;

            if (r.reqs)
                r.reqs->setItem(ecs, index, current, needed);
        });
    }

    void ActivityRow::setName(EntitySystem* ecs, const std::string& text)
    {
        onLive(this, [ecs, &text](ActivityRow& r) {
            r.spec.name = text;
            r.name.setText(ecs, text);
            r.fitName(ecs);

            // A tile's name may take another line, or one less
            if (r.spec.tile)
                r.resize(ecs);
        });
    }

    void ActivityRow::setGlyph(EntitySystem* ecs, const std::string& glyph)
    {
        onLive(this, [ecs, &glyph](ActivityRow& r) {
            r.spec.glyph = glyph;

            // A tile keeps the glyph and draws no mark
            if (not r.mark.entity.empty())
                r.mark.setName(ecs, glyph);
        });
    }

    void ActivityRow::setMonths(EntitySystem* ecs, int months)
    {
        onLive(this, [ecs, months](ActivityRow& r) {
            r.spec.months = std::max(0, months);
            r.cost.setText(ecs, costText(r.spec));
            r.fitName(ecs);

            // Done at once or not is its kind, and a compact row's ground says it
            r.root.get<ActivityRowState>()->instant = r.spec.months <= 0;
            paintRow(r);

            // A tile's name wraps beside the box of its time, which is another width now
            if (r.spec.tile)
                r.resize(ecs);
        });
    }

    void ActivityRow::setEach(EntitySystem* ecs, const std::string& text)
    {
        onLive(this, [ecs, &text](ActivityRow& r) {
            if (not r.each)
            {
                LOG_WARNING(DOM, "Row '" << r.spec.id << "' was built without an `each` line; setEach ignored");
                return;
            }

            r.spec.each = text;
            r.each->setText(ecs, text);
            r.fitName(ecs);
        });
    }

    void ActivityRow::setCount(EntitySystem* ecs, const std::string& text)
    {
        onLive(this, [ecs, &text](ActivityRow& r) {
            if (r.spec.count == text)
                return;

            // A compact row keeps the count for whoever says it (its gloss), and shows none
            if (r.spec.compact)
            {
                r.spec.count = text;
                return;
            }

            if (not r.rank)
            {
                LOG_WARNING(DOM, "Row '" << r.spec.id << "' was built without a rank or a count; setCount ignored");
                return;
            }

            r.spec.count = text;
            r.rank->setText(ecs, rankText(r.spec));
            r.fitName(ecs);
        });
    }

    void ActivityRow::setMinHeight(EntitySystem* ecs, float height)
    {
        onLive(this, [ecs, height](ActivityRow& r) {
            if (std::abs(r.spec.minHeight - height) < 0.01f)
                return;

            r.spec.minHeight = std::max(0.0f, height);
            r.resize(ecs);
        });
    }

    void ActivityRow::setUntil(EntitySystem* ecs, const std::string& text, bool urgent)
    {
        onLive(this, [ecs, &text, urgent](ActivityRow& r) {
            if (r.spec.until == text and r.spec.urgent == urgent)
                return;

            r.spec.until = text;
            r.spec.urgent = urgent;

            r.buildUntil(ecs);
            r.resize(ecs);
        });
    }

    void ActivityRow::setCloses(EntitySystem* ecs, int months, bool urgent)
    {
        onLive(this, [ecs, months, urgent](ActivityRow& r) {
            if (r.spec.closes == months and r.spec.urgent == urgent)
                return;

            r.spec.closes = months;
            r.spec.urgent = urgent;

            if (r.closing)
                r.closing->setText(ecs, closesText(r.spec));

            paintRow(r);
        });
    }

    void ActivityRow::setWidth(EntitySystem* ecs, float width)
    {
        onLive(this, [ecs, width](ActivityRow& r) {
            if (std::abs(r.spec.width - width) < 0.5f)
                return;

            r.spec.width = width;
            r.root.get<PositionComponent>()->setWidth(width);

            // The ground, rule, edge and ring follow the root through their anchors; the cost is
            // right-anchored. The name and the running rule take the room in between.
            r.fitName(ecs);

            if (r.progress)
                r.progress->setWidth(ecs, r.middleWidth());

            if (r.gains)
                r.gains->setWidth(ecs, r.middleWidth());

            // The caption or the gains may wrap differently at the new width
            r.resize(ecs);
        });
    }

    float ActivityRow::height(EntitySystem*) const
    {
        return root.get<PositionComponent>()->height;
    }

    EntityRef makeActivityHeading(EntitySystem* ecs, const std::string& text, float width, int z)
    {
        auto block = makeAnchoredPrefab(ecs, Offstage, Offstage, static_cast<float>(z));
        block.get<PositionComponent>()->setWidth(width);
        block.get<PositionComponent>()->setHeight(HeadingAbove + HeadingHeight + HeadingBelow);

        Label label = makeActivityText(ecs, "heading", "activity.group", text, z + 4);
        placeIn(label.entity, block.id, 0.0f, HeadingAbove, 4.0f);
        block.get<Prefab>()->addToPrefab(label.entity, "label");

        auto state = ecs->attach<ActivityHeadingState>(block.entity);
        state->label = label.entity.id;
        state->text = text;

        return block.entity;
    }

    namespace
    {
        constexpr float HeadHeight = 26.0f;       // A table's head: its columns' names over a rule

        // The head of a table of lines: the names of its columns, each over its column (the name
        // at the lines' left, the cost ending where their costs end, the cell where theirs
        // begins), and a rule under them. `inset` and `cell` are the lines' own
        EntityRef makeTableHead(EntitySystem* ecs, const ActivityListSpec& spec, const std::string& key, float inset, float cell, float action)
        {
            auto block = makeAnchoredPrefab(ecs, Offstage, Offstage, static_cast<float>(spec.z));
            block.get<PositionComponent>()->setWidth(spec.width);
            block.get<PositionComponent>()->setHeight(HeadHeight);

            const _unique_id id = block.id;
            auto prefab = block.get<Prefab>();

            auto head = [&](const std::string& text, bool left, float margin, float width) {
                if (text.empty())
                    return;

                Label label = makeActivityText(ecs, "label", "activity.head", text, spec.z + 4, width > 0.0f ? Overflow::Ellipsis : Overflow::Grow, width);
                auto anchor = label.entity->get<UiAnchor>();

                if (left)
                {
                    anchor->setLeftAnchor(PosAnchor{id, AnchorType::Left});
                    anchor->setLeftMargin(margin);
                }
                else
                {
                    anchor->setRightAnchor(PosAnchor{id, AnchorType::Right});
                    anchor->setRightMargin(margin);
                }

                anchor->setVerticalCenter(PosAnchor{id, AnchorType::VerticalCenter});
                anchor->setZConstrain(PosConstrain{id, AnchorType::Z, PosOpType::Add, 4.0f});
                prefab->addToPrefab(label.entity);
            };

            head(spec.headName, true, Pad, 0.0f);
            head(spec.headCost, false, Pad + inset, 0.0f);

            if (cell > 0.0f)
                head(spec.headAside, false, Pad + action, cell);

            auto rule = makeUiSimple2DShape(ecs, Shape2D::Square, spec.width, 1.0f);
            offstage(rule.entity);
            {
                auto anchor = rule.get<UiAnchor>();

                anchor->setLeftAnchor(PosAnchor{id, AnchorType::Left});
                anchor->setRightAnchor(PosAnchor{id, AnchorType::Right});
                anchor->setBottomAnchor(PosAnchor{id, AnchorType::Bottom});
                anchor->setZConstrain(PosConstrain{id, AnchorType::Z, PosOpType::Add, 1.0f});
            }
            rule.get<PositionComponent>()->setHeight(1.0f);
            ecs->attach<ThemeComponent>(rule.entity, "activity.row.rule");
            prefab->addToPrefab(rule.entity);

            auto state = ecs->attach<ActivityHeadingState>(block.entity);
            state->text = key;
            state->table = true;

            return block.entity;
        }
    }

    ActivityList makeActivityList(EntitySystem* ecs, const ActivityListSpec& spec)
    {
        if (not ecs->getSystem<ActivitySystem>())
        {
            LOG_ERROR(DOM, "makeActivityList needs an ActivitySystem in this ECS");
            return ActivityList{};
        }

        registerActivityComponents(ecs);

        const float W = spec.width;
        const float H = std::max(0.0f, spec.height);
        const bool scrolls = H > 0.0f;

        ActivityList list;
        list.spec = spec;
        list.spec.height = H;

        // Root: a prefab of its own, so a name given to the list resolves to it
        auto root = makeAnchoredPrefab(ecs, 0.0f, 0.0f, static_cast<float>(spec.z));
        root.get<PositionComponent>()->setWidth(W);
        list.root = root.entity;

        const _unique_id rootId = root.id;

        auto state = ecs->attach<ActivityListState>(root.entity);
        state->id = spec.id;
        state->stripes = spec.stripes;

        // Body: the rows stack here. With a height it keeps it, clips its rows and scrolls.
        auto body = makeVerticalLayout(ecs, 0.0f, 0.0f, W, H, scrolls);
        body.get<VerticalLayout>()->spacing = 0;
        body.get<VerticalLayout>()->scrollSpeed = ScrollStep;
        {
            auto anchor = body.get<UiAnchor>();

            anchor->setLeftAnchor(PosAnchor{rootId, AnchorType::Left});
            anchor->setRightAnchor(PosAnchor{rootId, AnchorType::Right});
            anchor->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
            anchor->setZConstrain(PosConstrain{rootId, AnchorType::Z});
        }
        root.get<Prefab>()->addToPrefab(body.entity, "body");
        list.body = body.entity;

        if (scrolls)
        {
            root.get<PositionComponent>()->setHeight(H);

            auto thumb = makeUiSimple2DShape(ecs, Shape2D::Square, ScrollWidth, 1.0f);
            thumb.get<UiAnchor>()->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 6.0f});
            thumb.get<PositionComponent>()->setVisibility(false);
            ecs->attach<ThemeComponent>(thumb.entity, "activity.scroll");
            root.get<Prefab>()->addToPrefab(thumb.entity, "scroll");
            list.scroll = thumb.entity;

            // The layout places the thumb on its right edge and sizes it to what is in view
            body.get<VerticalLayout>()->setVerticalScrollBar(thumb.entity);

            // A press and a drag scroll the rows; the release of a drag selects nothing
            body.get<VerticalLayout>()->dragToScroll = true;
        }
        else
        {
            root.get<UiAnchor>()->setHeightConstrain(PosConstrain{body.entity.id, AnchorType::Height});
        }

        // A table of lines has a drawn frame around it, over its rows
        if (spec.framed)
        {
            auto frame = makeStrokeRect2DShape(ecs, 1.0f, 1.0f, {255.0f, 255.0f, 255.0f, 255.0f}, 1.0f);
            frame.get<UiAnchor>()->fillIn(root.get<UiAnchor>());
            frame.get<UiAnchor>()->setZConstrain(PosConstrain{rootId, AnchorType::Z, PosOpType::Add, 5.0f});
            ecs->attach<ThemeComponent>(frame.entity, "activity.list.frame");
            root.get<Prefab>()->addToPrefab(frame.entity, "frame");
        }

        list.setRows(ecs, spec.groups);

        return list;
    }

    void ActivityList::select(EntitySystem* ecs, const std::string& id)
    {
        auto system = ecs->getSystem<ActivitySystem>();

        if (not system)
        {
            LOG_ERROR(DOM, "ActivityList::select needs an ActivitySystem in this ECS");
            return;
        }

        system->select(root, id);
    }

    const std::string& ActivityList::selected() const
    {
        return root.get<ActivityListState>()->selected;
    }

    ActivityRow* ActivityList::find(EntitySystem*, const std::string& id)
    {
        std::vector<EntityRef> rows;
        std::vector<EntityRef> headings;
        walkList(body, rows, headings);

        for (auto& entity : rows)
        {
            if (entity->get<ActivityRowState>()->id == id and entity->has<ActivityRow>())
                return entity->get<ActivityRow>().component;
        }

        return nullptr;
    }

    ActivityRow* ActivityList::row(EntitySystem* ecs, const std::string& id)
    {
        if (auto r = find(ecs, id))
            return r;

        LOG_ERROR(DOM, "List '" << spec.id << "' has no row '" << id << "'");

        return nullptr;
    }

    void ActivityList::setRowState(EntitySystem* ecs, const std::string& id, ActivityState newState)
    {
        if (auto r = row(ecs, id))
            r->setState(ecs, newState);
    }

    namespace
    {
        bool sameGains(const std::vector<Gain>& a, const std::vector<Gain>& b)
        {
            if (a.size() != b.size())
                return false;

            for (size_t i = 0; i < a.size(); ++i)
            {
                if (a[i].stat != b[i].stat or a[i].amount != b[i].amount)
                    return false;
            }

            return true;
        }

        bool sameRequirements(const std::vector<Requirement>& a, const std::vector<Requirement>& b)
        {
            if (a.size() != b.size())
                return false;

            for (size_t i = 0; i < a.size(); ++i)
            {
                if (a[i].label != b[i].label or a[i].current != b[i].current or a[i].needed != b[i].needed or a[i].met != b[i].met)
                    return false;
            }

            return true;
        }

        bool near(float a, float b) { return std::abs(a - b) < 0.01f; }

        // Whether a row that stands would be built the same from another spec: everything that is
        // said of a row. Its width is not: a row kept is given the width it should have. Neither
        // is what the list sets on its own (its z, its stripe, the height of its line)
        bool sameRow(const ActivityRowSpec& held, const ActivityRowSpec& wanted)
        {
            return held.id == wanted.id and held.name == wanted.name and held.glyph == wanted.glyph and held.rank == wanted.rank
               and held.count == wanted.count and held.months == wanted.months and held.each == wanted.each
               and held.until == wanted.until and held.urgent == wanted.urgent and held.compact == wanted.compact
               and held.featured == wanted.featured and held.major == wanted.major and held.tile == wanted.tile
               and held.state == wanted.state and held.caption == wanted.caption and held.glossKey == wanted.glossKey
               and held.cost == wanted.cost and held.costGlyph == wanted.costGlyph
               and held.closes == wanted.closes and held.adventure == wanted.adventure
               and held.line == wanted.line and held.eachTone == wanted.eachTone and held.asideTop == wanted.asideTop
               and held.asideBottom == wanted.asideBottom and held.asideTopTone == wanted.asideTopTone
               and held.asideBottomTone == wanted.asideBottomTone and near(held.asideWidth, wanted.asideWidth)
               and held.action == wanted.action and held.actionTag == wanted.actionTag
               and near(held.percent, wanted.percent) and near(held.glideTo, wanted.glideTo) and near(held.glideMs, wanted.glideMs)
               and sameGains(held.gains, wanted.gains) and sameRequirements(held.requirements, wanted.requirements);
        }

        // A tile taken off its line without being destroyed: the line no longer answers for it
        void unhook(EntityRef line, EntityRef tile)
        {
            if (line.empty())
                return;

            if (line->has<Prefab>())
                line->get<Prefab>()->childrenIds.erase(tile.id);
        }
    }

    void ActivityList::setRows(EntitySystem* ecs, const std::vector<ActivityGroup>& groups)
    {
        auto layout = body.get<VerticalLayout>();
        auto state = root.get<ActivityListState>();

        // What the body holds, top to bottom: what the fills before this one handed to it (the
        // layout takes what it is given a pass later, so a list filled twice in one pass does not
        // hold the first fill yet), and whatever else stands in it (a list filled by a page's file)
        std::vector<EntityRef> current = state->items;

        for (auto& child : layout->entities)
        {
            if (std::find_if(current.begin(), current.end(), [&](const EntityRef& item) { return item.id == child.id; }) == current.end())
                current.push_back(child);
        }

        const int perLine = columns();
        const float tile = tileSize();
        const bool tiles = perLine > 0;

        // What is wanted in its place, top to bottom: a heading, a row, or a line of tiles
        struct Wanted
        {
            bool heading = false;
            std::string label;
            std::vector<ActivityRowSpec> rows;       // The one row, or the tiles of the line left to right
        };

        std::vector<Wanted> wanted;

        // A table has its head over its lines, inside its frame: its columns are the first
        // line's (they all have the same)
        float headInset = 0.0f;
        float headCell = 0.0f;
        float headAction = 0.0f;
        std::string headKey;

        if (not tiles and not (spec.headName + spec.headCost + spec.headAside).empty())
        {
            for (const auto& group : groups)
            {
                if (group.rows.empty() or not headKey.empty())
                    continue;

                const ActivityRowSpec& first = group.rows.front();

                headAction = first.action.empty() ? 0.0f : ActionWidth + CellGap;
                headCell = first.asideTop.empty() and first.asideBottom.empty() ? 0.0f : first.asideWidth;
                headInset = headAction + (headCell > 0.0f ? headCell + CellGap : 0.0f);
                headKey = "\x01head|" + spec.headName + "|" + spec.headCost + "|" + spec.headAside + "|" + std::to_string(static_cast<int>(headInset)) + "|" + std::to_string(static_cast<int>(headCell));
            }

            if (not headKey.empty())
                wanted.push_back({true, headKey, {}});
        }

        for (const auto& group : groups)
        {
            if (not group.label.empty())
                wanted.push_back({true, group.label, {}});

            // Tiles: as many to a line as fit, a new line when one is full and with every group
            bool open = false;

            for (auto rowSpec : group.rows)
            {
                rowSpec.z = spec.z;
                rowSpec.compact = rowSpec.compact or spec.compact;
                rowSpec.width = tiles ? tile : spec.width;

                if (tiles)
                {
                    rowSpec.tile = true;
                    rowSpec.minHeight = 0.0f;
                }

                // As a row stores what it was made from (makeActivityRow): what stands is compared with this
                rowSpec.months = std::max(0, rowSpec.months);
                rowSpec.percent = std::clamp(rowSpec.percent, 0.0f, 100.0f);
                rowSpec.compact = rowSpec.compact or rowSpec.tile;

                if (not tiles or not open or static_cast<int>(wanted.back().rows.size()) == perLine)
                {
                    wanted.push_back({false, "", {}});
                    open = true;
                }

                wanted.back().rows.push_back(rowSpec);
            }
        }

        // The rows that stand, by their id: each is kept if it would be built the same
        struct Held
        {
            EntityRef entity;
            EntityRef line;              // Empty for a row that stands in the body itself
            size_t slot = 0;             // Its place on its line
            bool used = false;
        };

        std::unordered_map<std::string, Held> held;

        for (auto& item : current)
        {
            if (hasComp<ActivityLineState>(item))
            {
                auto& onLine = item->get<ActivityLineState>()->tiles;

                for (size_t i = 0; i < onLine.size(); ++i)
                {
                    if (hasComp<ActivityRow>(onLine[i]))
                        held[onLine[i]->get<ActivityRow>()->spec.id] = Held{onLine[i], item, i};
                }
            }
            else if (hasComp<ActivityRow>(item))
            {
                held[item->get<ActivityRow>()->spec.id] = Held{item, EntityRef{}, 0};
            }
        }

        // From the top, what stands is kept while it is what is wanted there: a heading that
        // reads the same, a row that would be built the same, a line of tiles (its tiles are
        // seen to one by one). From the first that is not, what stands goes and the rest is made
        size_t keep = 0;

        for (; keep < current.size() and keep < wanted.size(); ++keep)
        {
            EntityRef item = current[keep];   // By value: a handle is asked for its components, which a const one cannot be
            const Wanted& want = wanted[keep];

            if (want.heading)
            {
                if (not hasComp<ActivityHeadingState>(item) or item->get<ActivityHeadingState>()->text != want.label)
                    break;
            }
            else if (tiles)
            {
                if (not hasComp<ActivityLineState>(item))
                    break;
            }
            else if (not hasComp<ActivityRow>(item) or not sameRow(item->get<ActivityRow>()->spec, want.rows.front()))
            {
                break;
            }
        }

        std::vector<EntityRef> items(current.begin(), current.begin() + static_cast<std::ptrdiff_t>(keep));
        std::vector<EntityRef> lines;

        // The lines there are let go of their tiles: each is given the ones it should hold below
        for (auto& item : current)
        {
            if (hasComp<ActivityLineState>(item))
                item->get<ActivityLineState>()->tiles.clear();
        }

        // A tile for a place on a line: the one that stands if it would be built the same, left
        // where it is or brought there, at the width it should have; a new one otherwise, which
        // takes the selection of the one it replaces when it can be chosen
        auto seat = [&](EntityRef line, const ActivityRowSpec& rowSpec, size_t slot) {
            const float x = static_cast<float>(slot) * (tile + TileGap);
            auto it = held.find(rowSpec.id);

            if (it != held.end() and not it->second.used and sameRow(it->second.entity->get<ActivityRow>()->spec, rowSpec))
            {
                Held& h = it->second;
                h.used = true;

                const bool resized = std::abs(h.entity->get<ActivityRow>()->spec.width - rowSpec.width) > 0.01f;

                h.entity->get<ActivityRow>()->setWidth(ecs, rowSpec.width);

                if (h.line.id == line.id)
                {
                    // On its line still: moved along it at most, by a tile that left before it or
                    // by tiles of another width
                    if (h.slot != slot or resized)
                        h.entity->get<UiAnchor>()->setLeftMargin(x);

                    line->get<ActivityLineState>()->tiles.push_back(h.entity);
                }
                else
                {
                    unhook(h.line, h.entity);
                    placeTile(line, h.entity, x);
                }

                return;
            }

            EntityRef made = makeActivityRow(ecs, rowSpec).root;

            if (it != held.end() and not it->second.used and rowSpec.state == ActivityState::Idle
                and it->second.entity->get<ActivityRowState>()->selected and state->selected == rowSpec.id)
            {
                made->get<ActivityRowState>()->selected = true;
            }

            placeTile(line, made, x);
        };

        // The lines kept: their tiles
        for (size_t i = 0; i < keep; ++i)
        {
            if (wanted[i].heading or not tiles)
                continue;

            for (size_t slot = 0; slot < wanted[i].rows.size(); ++slot)
                seat(current[i], wanted[i].rows[slot], slot);

            lines.push_back(current[i]);
        }

        // A row kept in the body itself stays as it is, at the width it should have
        if (not tiles)
        {
            for (size_t i = 0; i < keep; ++i)
            {
                if (wanted[i].heading)
                    continue;

                if (auto it = held.find(wanted[i].rows.front().id); it != held.end())
                    it->second.used = true;

                current[i]->get<ActivityRow>()->setWidth(ecs, spec.width);
            }
        }

        // What is wanted past them: made, and handed to the body
        std::vector<EntityRef> made;

        for (size_t i = keep; i < wanted.size(); ++i)
        {
            const Wanted& want = wanted[i];
            EntityRef item;

            if (want.heading)
            {
                item = want.label == headKey and not headKey.empty() ? makeTableHead(ecs, spec, headKey, headInset, headCell, headAction)
                                                                     : makeActivityHeading(ecs, want.label, spec.width, spec.z);
            }
            else if (tiles)
            {
                item = makeTileLine(ecs, spec.width, spec.z);

                for (size_t slot = 0; slot < want.rows.size(); ++slot)
                    seat(item, want.rows[slot], slot);

                lines.push_back(item);
            }
            else
            {
                item = makeActivityRow(ecs, want.rows.front()).root;

                auto it = held.find(want.rows.front().id);

                if (it != held.end() and not it->second.used and want.rows.front().state == ActivityState::Idle
                    and it->second.entity->get<ActivityRowState>()->selected and state->selected == want.rows.front().id)
                {
                    item->get<ActivityRowState>()->selected = true;
                }
            }

            made.push_back(item);
        }

        // A tile no line took goes. Its line does not answer for it any more: a line that stays
        // would keep a dead tile, and one that goes would destroy it a second time
        if (tiles)
        {
            for (auto& [id, h] : held)
            {
                if (h.used or h.line.empty())
                    continue;

                unhook(h.line, h.entity);
                ecs->removeEntity(h.entity);
            }
        }

        // What stood past what was kept goes: the layout destroys each one as it lets it go
        // (destroying them here as well would leave the layout holding dead rows until it gets to
        // the removal, and the list walks those rows every frame)
        for (size_t i = keep; i < current.size(); ++i)
            layout->removeEntity(current[i].id);

        for (auto& item : made)
        {
            layout->addEntity(item);
            items.push_back(item);
        }

        // A heading or a line that was kept is as wide as the list is now
        for (size_t i = 0; i < keep; ++i)
        {
            if (wanted[i].heading or tiles)
                current[i]->get<PositionComponent>()->setWidth(spec.width);
        }

        for (auto& line : lines)
            levelLine(ecs, line);

        state->items = items;
        state->given.clear();

        for (auto& item : items)
            state->given.push_back(item.id);

        spec.groups = groups;
    }

    int ActivityList::columns() const
    {
        if (spec.tileWidth <= 0.0f)
            return 0;

        // One more tile needs its width and the gap before it
        return std::max(1, static_cast<int>(std::floor((spec.width + TileGap) / (spec.tileWidth + TileGap))));
    }

    float ActivityList::tileSize() const
    {
        const int perLine = columns();

        if (perLine <= 0)
            return spec.width;

        return (spec.width - static_cast<float>(perLine - 1) * TileGap) / static_cast<float>(perLine);
    }

    void ActivityList::setSize(EntitySystem* ecs, float width, float height)
    {
        const bool widened = std::abs(spec.width - width) > 0.5f;

        spec.width = width;
        root.get<PositionComponent>()->setWidth(width);   // The body is anchored to both sides

        // A list that scrolls keeps the height it is given; one that does not is as tall as its rows
        if (spec.height > 0.0f and height > 0.0f)
        {
            spec.height = height;
            root.get<PositionComponent>()->setHeight(height);
            body.get<PositionComponent>()->setHeight(height);
        }

        std::vector<EntityRef> rows;
        std::vector<EntityRef> headings;
        walkList(body, rows, headings);

        // Tiles: another width is another number to a line, or other tiles. They are laid again,
        // each as it stands now (its state, its closing), the selection let go
        if (spec.tileWidth > 0.0f)
        {
            if (not widened)
                return;

            std::vector<ActivityGroup> groups = spec.groups;

            for (auto& group : groups)
            {
                for (auto& row : group.rows)
                {
                    for (auto& entity : rows)
                    {
                        if (entity->has<ActivityRow>() and entity->get<ActivityRow>()->spec.id == row.id)
                            row = entity->get<ActivityRow>()->spec;
                    }
                }
            }

            setRows(ecs, groups);

            return;
        }

        for (auto& entity : rows)
        {
            if (entity->has<ActivityRow>())
                entity->get<ActivityRow>()->setWidth(ecs, width);
        }

        for (auto& heading : headings)
            heading->get<PositionComponent>()->setWidth(width);

        for (auto& group : spec.groups)
        {
            for (auto& row : group.rows)
                row.width = width;
        }
    }

    void ActivitySystem::init()
    {
        registerActivityComponents(ecsRef);

        auto group = registerGroup<ActivityRowState>();

        group->addOnGroup([this](EntityRef entity) {
            if (not entity->has<MouseEnterComponent>())
                ecsRef->attach<MouseEnterComponent>(entity, makeCallable<ActivityNoOp>(ActivityNoOp{}));

            if (not entity->has<MouseLeaveComponent>())
                ecsRef->attach<MouseLeaveComponent>(entity, makeCallable<ActivityNoOp>(ActivityNoOp{}));
        });

        // A removed row leaves the Tab order with it.
        group->removeOfGroup([this](EntitySystem*, _unique_id id) {
            if (auto focusOrder = ecsRef->getSystem<FocusOrderSystem>())
                focusOrder->remove(id);

            if (lastReleased == id)
                lastReleased = 0;
        });
    }

    void ActivitySystem::applyVisual(EntityRef root)
    {
        // Through the row's refs when it has them, so a row still pending in the pool paints too.
        if (not root.empty() and root->has<ActivityRow>())
            paintRow(*root->get<ActivityRow>().component);
        else
            paint(ecsRef, root);
    }

    void ActivitySystem::adopt(EntityRef listRoot)
    {
        auto list = listRoot->get<ActivityListState>();

        EntityRef body = listRoot->get<Prefab>()->getEntity("body");

        std::vector<EntityRef> rows;
        std::vector<EntityRef> headings;
        walkList(body, rows, headings);

        std::vector<_unique_id> rowIds;
        std::vector<_unique_id> headingIds;

        for (auto& row : rows)
            rowIds.push_back(row.id);

        for (auto& heading : headings)
            headingIds.push_back(heading.id);

        if (rowIds == list->rows and headingIds == list->headings)
            return;

        list->rows = rowIds;
        list->headings = headingIds;

        // Other rows than a frame ago, and the mouse may not have moved: the one now under it is
        // hovered only by a move. Asked for again while they are being placed, a row begun by a
        // double click takes the next click where the mouse stands
        hoverStale = HoverFrames;

        bool selectionFound = list->selected.empty();

        // Stripes alternate across the whole list, not per group: the second, fourth... rows.
        for (size_t i = 0; i < rows.size(); ++i)
        {
            auto& entity = rows[i];
            auto st = entity->get<ActivityRowState>();

            st->list = listRoot.id;
            st->stripe = list->stripes and i % 2 == 1;
            st->last = i + 1 == rows.size();

            if (entity->has<ActivityRow>())
                entity->get<ActivityRow>()->spec.stripe = st->stripe;

            if (st->selected and st->id == list->selected)
                selectionFound = true;

            applyVisual(entity);
        }

        if (not selectionFound)
            list->selected.clear();

        for (size_t i = 0; i < headings.size(); ++i)
        {
            auto& entity = headings[i];

            // A table's head is not a group's heading: it is as it was made
            if (entity->get<ActivityHeadingState>()->table)
                continue;

            const float above = i == 0 ? 0.0f : HeadingAbove;

            entity->get<PositionComponent>()->setHeight(above + HeadingHeight + HeadingBelow);

            EntityRef label = entity->get<Prefab>()->getEntity("label");

            if (not label.empty())
                label->get<UiAnchor>()->setTopMargin(above);
        }
    }

    void ActivitySystem::execute()
    {
        for (auto list : view<ActivityListState>())
        {
            if (auto entity = ecsRef->getEntity(list->entityId))
            {
                adopt(entity);
                level(entity);
            }
        }

        if (hoverStale > 0)
        {
            --hoverStale;

            ecsRef->sendEvent(RefreshHoverEvent{});
        }
    }

    void ActivitySystem::level(EntityRef listRoot)
    {
        EntityRef body = listRoot->get<Prefab>()->getEntity("body");

        if (body.empty() or not body->has<VerticalLayout>())
            return;

        // A tile that gained or lost a line (its closing, its name) changes what its line asks
        for (auto& child : body->get<VerticalLayout>()->entities)
        {
            if (hasComp<ActivityLineState>(child))
                levelLine(ecsRef, child);
        }
    }

    bool ActivitySystem::select(EntityRef listRoot, const std::string& id)
    {
        if (listRoot.empty() or not listRoot->has<ActivityListState>())
        {
            LOG_ERROR(DOM, "select('" << id << "'): not an activity list");
            return false;
        }

        auto list = listRoot->get<ActivityListState>();

        ActivityRowState* target = nullptr;

        if (not id.empty())
        {
            for (auto st : view<ActivityRowState>())
            {
                if (st->list == listRoot.id and st->id == id)
                    target = st;
            }

            if (not target)
            {
                LOG_ERROR(DOM, "List '" << list->id << "' has no row '" << id << "'");
                return false;
            }

            if (target->state != ActivityState::Idle)
            {
                LOG_WARNING(DOM, "Row '" << id << "' is " << (target->state == ActivityState::Locked ? "locked" : "running") << "; it cannot be selected");
                return false;
            }
        }

        if (list->selected == id)
            return true;

        for (auto st : view<ActivityRowState>())
        {
            if (st->list != listRoot.id)
                continue;

            const bool selected = st == target;

            if (st->selected != selected)
            {
                st->selected = selected;
                applyVisual(ecsRef->getEntity(st->entityId));
            }
        }

        list->selected = id;
        ecsRef->sendEvent(ActivitySelectedEvent{list->id, id});

        return true;
    }

    void ActivitySystem::activate(EntityRef row)
    {
        auto st = row->get<ActivityRowState>();

        if (st->state != ActivityState::Idle)
            return;

        std::string listId;

        if (auto listEntity = ecsRef->getEntity(st->list); listEntity and listEntity->has<ActivityListState>())
            listId = listEntity->get<ActivityListState>()->id;

        ecsRef->sendEvent(ActivityActivatedEvent{listId, st->id});
    }

    void ActivitySystem::onEvent(const HoverChangedEvent& event)
    {
        // The list of a row, by its id, for the event that says what the mouse is on
        auto listOf = [this](_unique_id listId) -> std::string {
            if (auto listEntity = ecsRef->getEntity(listId); listEntity and listEntity->has<ActivityListState>())
                return listEntity->get<ActivityListState>()->id;

            return "";
        };

        bool changed = false;
        std::string list, on;

        // Locked rows take the hover, so nothing under them lights, but they never change.
        for (auto id : event.entered)
        {
            if (auto entity = ecsRef->getEntity(id); entity and entity->has<ActivityRowState>())
            {
                auto st = entity->get<ActivityRowState>();
                st->hovered = true;
                applyVisual(entity);

                changed = true;
                list = listOf(st->list);
                on = st->id;
            }
        }

        for (auto id : event.left)
        {
            if (auto entity = ecsRef->getEntity(id); entity and entity->has<ActivityRowState>())
            {
                auto st = entity->get<ActivityRowState>();
                st->hovered = false;
                st->pressed = false;   // A drag off cancels
                applyVisual(entity);

                // Off this row and onto none: a row entered in the same move has the last word
                if (on.empty())
                {
                    changed = true;
                    list = listOf(st->list);
                }
            }
        }

        if (changed)
            ecsRef->sendEvent(ActivityHoveredEvent{list, on});
    }

    void ActivitySystem::onEvent(const OnMouseClick& event)
    {
        if (event.button != 1)
            return;

        for (auto st : view<ActivityRowState>())
        {
            if (st->hovered and st->state == ActivityState::Idle)
                st->pressed = true;
        }
    }

    void ActivitySystem::onEvent(const OnMouseRelease& event)
    {
        if (event.button != 1)
            return;

        _unique_id released = 0;

        for (auto st : view<ActivityRowState>())
        {
            if (not st->pressed)
                continue;

            st->pressed = false;

            if (st->hovered)
                released = st->entityId;
        }

        // The press scrolled the list: it was not a click
        if (released == 0 or event.cancelled)
            return;

        auto row = ecsRef->getEntity(released);
        auto st = row->get<ActivityRowState>();

        // A second release on the selected row confirms it; a first one selects it.
        if (st->selected and lastReleased == released and now - lastReleaseAt <= DoubleReleaseMs)
            activate(row);
        else if (not st->selected and st->list != 0)
            select(ecsRef->getEntity(st->list), st->id);

        lastReleased = released;
        lastReleaseAt = now;
    }

    void ActivitySystem::onEvent(const OnSDLScanCode& event)
    {
        if (event.key != SDL_SCANCODE_RETURN and event.key != SDL_SCANCODE_SPACE)
            return;

        auto focusOrder = ecsRef->getSystem<FocusOrderSystem>();

        // Only a row the keyboard is on (its ring showing): after a click the focus order still
        // remembers the row, and a key the scene uses (Space) must not select it behind its back.
        if (not focusOrder or not focusOrder->keyboardFocus())
            return;

        auto row = ecsRef->getEntity(focusOrder->current());

        if (not row or not row->has<ActivityRowState>())
            return;

        auto st = row->get<ActivityRowState>();

        if (st->state != ActivityState::Idle)
            return;

        // Enter on the selected row confirms it; on another row it selects that one first.
        if (st->selected)
            activate(row);
        else if (st->list != 0)
            select(ecsRef->getEntity(st->list), st->id);
    }

    void ActivitySystem::onEvent(const KeyboardFocusChangedEvent& event)
    {
        for (auto st : view<ActivityRowState>())
        {
            const bool focused = st->entityId == event.face and event.keyboard;

            if (st->keyboardFocus != focused)
            {
                st->keyboardFocus = focused;
                applyVisual(ecsRef->getEntity(st->entityId));
            }
        }
    }

    void ActivitySystem::onEvent(const TickEvent& event)
    {
        now += event.tick;
    }
}
