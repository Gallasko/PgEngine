#include "radiocard.h"

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL.h>
#elif __linux__
#include <SDL2/SDL.h>
#elif _WIN32
#include <SDL.h>
#endif

#include <algorithm>
#include <cmath>

#include "ECS/callable.h"
#include "2D/position.h"
#include "2D/simple2dobject.h"
#include "2D/decoratedshapes.h"
#include "UI/prefab.h"
#include "UI/ttftext.h"
#include "UI/themesystem.h"

using namespace pg;

namespace chronicle
{
    namespace
    {
        struct RadioNoOp {};

        constexpr float CARD_H = 64.0f;
        constexpr float PAD = 12.0f;          // space-3 inside a card
        constexpr float GAP = 12.0f;          // space-3 between two cards
        constexpr float RADIO = 18.0f;
        constexpr float DOT = 8.0f;
        constexpr float EDGE = 3.0f;          // The chosen card's left edge
        constexpr float SWATCH = 40.0f;
        constexpr float LINE_H = 20.0f;       // control and body-sm lines
        constexpr float MIN_CARD = 260.0f;    // Narrower, the cards stand one under the other

        // The swatch's three strokes of writing: left, top, width; the last one in red lead
        struct Stroke { float x, y, w; const char* token; };
        constexpr Stroke Strokes[3] = {{8.0f, 10.0f, 24.0f, "ink"}, {8.0f, 17.0f, 18.0f, "ink"}, {8.0f, 24.0f, 22.0f, "vermilion"}};

        float textLeft(bool swatch) { return PAD + RADIO + PAD + (swatch ? SWATCH + PAD : 0.0f); }

        void anchorIn(UiAnchor* anchor, _unique_id face, float left, float top, float z)
        {
            anchor->setLeftAnchor(PosAnchor{face, AnchorType::Left}); anchor->setLeftMargin(left);
            anchor->setTopAnchor(PosAnchor{face, AnchorType::Top}); anchor->setTopMargin(top);
            anchor->setZConstrain(PosConstrain{face, AnchorType::Z, PosOpType::Add, z});
        }
    }

    RadioCards makeRadioCards(EntitySystem* ecs, const RadioCardsSpec& spec)
    {
        auto* theme = ecs->getSystem<ThemeSystem>();
        const int z = spec.z;

        RadioCards rc;
        rc.spec = spec;

        auto root = makeAnchoredPrefab(ecs, 0.0f, 0.0f, static_cast<float>(z));
        const _unique_id rootId = root.id;
        rc.root = root.entity;

        auto add = [&root](EntityRef e) { root.get<Prefab>()->addToPrefab(e); };

        for (size_t i = 0; i < spec.items.size(); ++i)
        {
            const RadioCardItem& item = spec.items[i];
            const bool chosen = item.id == spec.selected;
            const bool hasSwatch = not item.swatch.empty() and theme->theme().hasThemeId(item.swatch);

            RadioCards::Card card;

            auto face = ecs->createEntity();
            auto fp = ecs->attach<PositionComponent>(face);
            fp->setHeight(CARD_H); fp->setZ(static_cast<float>(z));
            auto fa = ecs->attach<UiAnchor>(face);
            fa->setLeftAnchor(PosAnchor{rootId, AnchorType::Left});
            fa->setTopAnchor(PosAnchor{rootId, AnchorType::Top});
            fa->setZConstrain(PosConstrain{rootId, AnchorType::Z});
            ecs->attach<FocusableComponent>(face);
            auto st = ecs->attach<RadioCardState>(face);
            st->cards = rootId; st->index = static_cast<int>(i); st->id = item.id; st->tag = spec.tag; st->selected = chosen;
            const _unique_id faceId = face.id;
            root.get<Prefab>()->addToPrefab(face, "card" + std::to_string(i));
            card.face = face;

            if (auto* fo = ecs->getSystem<FocusOrderSystem>())
                fo->add(faceId);

            auto ground = makeUiSimple2DShape(ecs, Shape2D::Square, 1.0f, 1.0f);
            ground.get<UiAnchor>()->fillIn(fa);
            ground.get<UiAnchor>()->setZConstrain(PosConstrain{faceId, AnchorType::Z});
            ecs->attach<ThemeComponent>(ground.entity, "radio.ground");
            add(ground.entity);
            card.ground = ground.entity;
            st->ground = ground.entity.id;

            auto frame = makeStrokeRect2DShape(ecs, 1.0f, 1.0f, {255.0f, 255.0f, 255.0f, 255.0f}, 2.0f);
            frame.get<UiAnchor>()->fillIn(fa);
            frame.get<UiAnchor>()->setZConstrain(PosConstrain{faceId, AnchorType::Z, PosOpType::Add, 1.0f});
            ecs->attach<ThemeComponent>(frame.entity, "radio.frame");
            add(frame.entity);
            card.frame = frame.entity;

            auto edge = makeUiSimple2DShape(ecs, Shape2D::Square, EDGE, CARD_H);
            anchorIn(edge.get<UiAnchor>().component, faceId, 0.0f, 0.0f, 2.0f);
            edge.get<PositionComponent>()->setVisible(chosen);
            ecs->attach<ThemeComponent>(edge.entity, "radio.edge");
            add(edge.entity);
            card.edge = edge.entity;
            st->edge = edge.entity.id;

            auto radio = makeStrokeRect2DShape(ecs, RADIO, RADIO, {255.0f, 255.0f, 255.0f, 255.0f}, 2.0f);
            anchorIn(radio.get<UiAnchor>().component, faceId, PAD, (CARD_H - RADIO) / 2.0f, 2.0f);
            ecs->attach<ThemeComponent>(radio.entity, chosen ? "radio.mark.on" : "radio.mark");
            add(radio.entity);
            card.radio = radio.entity;
            st->radio = radio.entity.id;

            auto dot = makeRoundedRect2DShape(ecs, DOT / 2.0f, DOT, DOT);
            anchorIn(dot.get<UiAnchor>().component, faceId, PAD + (RADIO - DOT) / 2.0f, (CARD_H - DOT) / 2.0f, 3.0f);
            dot.get<PositionComponent>()->setVisible(chosen);
            ecs->attach<ThemeComponent>(dot.entity, "radio.dot");
            add(dot.entity);
            card.dot = dot.entity;
            st->dot = dot.entity.id;

            // The swatch is the colours of its own theme, whichever is in use: painted here, from the
            // tokens, and never by the theme system (it carries no theme element). Its frame is the page's
            if (hasSwatch)
            {
                const float left = PAD + RADIO + PAD;
                const float top = (CARD_H - SWATCH) / 2.0f;

                auto swatch = makeUiSimple2DShape(ecs, Shape2D::Square, SWATCH, SWATCH, theme->theme().color("vellum", item.swatch));
                anchorIn(swatch.get<UiAnchor>().component, faceId, left, top, 2.0f);
                add(swatch.entity);
                card.swatch = swatch.entity;

                for (const auto& stroke : Strokes)
                {
                    auto line = makeUiSimple2DShape(ecs, Shape2D::Square, stroke.w, 2.0f, theme->theme().color(stroke.token, item.swatch));
                    anchorIn(line.get<UiAnchor>().component, faceId, left + stroke.x, top + stroke.y, 3.0f);
                    add(line.entity);
                    card.swatchParts.push_back(line.entity);
                }

                auto edgeOf = makeStrokeRect2DShape(ecs, SWATCH, SWATCH, {255.0f, 255.0f, 255.0f, 255.0f}, 1.0f);
                anchorIn(edgeOf.get<UiAnchor>().component, faceId, left, top, 4.0f);
                ecs->attach<ThemeComponent>(edgeOf.entity, "radio.swatch.frame");
                add(edgeOf.entity);
                card.swatchParts.push_back(edgeOf.entity);
            }

            const float tx = textLeft(hasSwatch);

            LabelSpec ls; ls.style = "control"; ls.text = item.label; ls.overflow = Overflow::Ellipsis; ls.width = 100.0f; ls.z = z + 3;
            card.label = makeLabel(ecs, ls);
            anchorIn(card.label.entity->get<UiAnchor>().component, faceId, tx, PAD, 3.0f);
            add(card.label.entity);

            LabelSpec ns; ns.style = "body-sm"; ns.color = "ink-muted"; ns.text = item.line; ns.overflow = Overflow::Ellipsis; ns.width = 100.0f; ns.z = z + 3;
            card.line = makeLabel(ecs, ns);
            anchorIn(card.line.entity->get<UiAnchor>().component, faceId, tx, PAD + LINE_H, 3.0f);
            add(card.line.entity);

            // ring: 2 px focus-ink, 4 px outside the card
            auto ring = makeStrokeRect2DShape(ecs, 1.0f, 1.0f, {255.0f, 255.0f, 255.0f, 255.0f}, theme->border("border-rule"));
            auto ra = ring.get<UiAnchor>();
            ra->setLeftAnchor(PosAnchor{faceId, AnchorType::Left});     ra->setLeftMargin(-4.0f);
            ra->setRightAnchor(PosAnchor{faceId, AnchorType::Right});   ra->setRightMargin(-4.0f);
            ra->setTopAnchor(PosAnchor{faceId, AnchorType::Top});       ra->setTopMargin(-4.0f);
            ra->setBottomAnchor(PosAnchor{faceId, AnchorType::Bottom}); ra->setBottomMargin(-4.0f);
            ra->setZConstrain(PosConstrain{faceId, AnchorType::Z, PosOpType::Add, 4.0f});
            ring.get<PositionComponent>()->setVisible(false);
            ecs->attach<ThemeComponent>(ring.entity, "radio.ring");
            add(ring.entity);
            card.ring = ring.entity;
            st->ring = ring.entity.id;

            rc.cards.push_back(card);
        }

        rc.setWidth(ecs, spec.width);

        return rc;
    }

    float RadioCards::height() const
    {
        if (cards.empty())
            return 0.0f;

        const bool sideBySide = spec.width >= 2.0f * MIN_CARD + GAP;
        const size_t rows = sideBySide ? (cards.size() + 1) / 2 : cards.size();

        return static_cast<float>(rows) * CARD_H + static_cast<float>(rows - 1) * GAP;
    }

    void RadioCards::setWidth(EntitySystem* ecs, float width)
    {
        spec.width = width;

        // Two to a line where they have room, else one
        const bool sideBySide = width >= 2.0f * MIN_CARD + GAP;
        const float cardW = sideBySide ? std::floor((width - GAP) / 2.0f) : width;

        for (size_t i = 0; i < cards.size(); ++i)
        {
            Card& card = cards[i];
            const size_t col = sideBySide ? i % 2 : 0;
            const size_t row = sideBySide ? i / 2 : i;

            auto anchor = card.face->get<UiAnchor>();
            anchor->setLeftMargin(static_cast<float>(col) * (cardW + GAP));
            anchor->setTopMargin(static_cast<float>(row) * (CARD_H + GAP));

            card.face->get<PositionComponent>()->setWidth(cardW);

            const float textW = std::max(1.0f, cardW - textLeft(not card.swatch.empty()) - PAD);

            card.label.setWidth(ecs, textW);
            card.line.setWidth(ecs, textW);
        }

        root->get<PositionComponent>()->setWidth(width);
        root->get<PositionComponent>()->setHeight(height());
    }

    void RadioCards::select(EntitySystem* ecs, const std::string& id)
    {
        spec.selected = id;

        auto* sys = ecs->getSystem<RadioCardSystem>();

        for (auto& card : cards)
        {
            auto st = card.face->get<RadioCardState>();
            st->selected = st->id == id;

            if (sys)
                sys->applyVisual(card.face);
        }
    }

    // ── RadioCardSystem ────────────────────────────────────────────────────────
    void RadioCardSystem::init()
    {
        auto group = registerGroup<RadioCardState>();
        group->addOnGroup([this](EntityRef entity) {
            if (not entity->has<MouseEnterComponent>())
                ecsRef->attach<MouseEnterComponent>(entity, makeCallable<RadioNoOp>(RadioNoOp{}));
            if (not entity->has<MouseLeaveComponent>())
                ecsRef->attach<MouseLeaveComponent>(entity, makeCallable<RadioNoOp>(RadioNoOp{}));
        });
    }

    void RadioCardSystem::applyVisual(EntityRef face)
    {
        auto st = face->get<RadioCardState>();

        if (auto e = ecsRef->getEntity(st->ground))
            e->get<ThemeComponent>()->setElement(st->hovered ? "radio.ground.hover" : "radio.ground");

        if (auto e = ecsRef->getEntity(st->edge))
            e->get<PositionComponent>()->setVisible(st->selected);

        if (auto e = ecsRef->getEntity(st->dot))
            e->get<PositionComponent>()->setVisible(st->selected);

        if (auto e = ecsRef->getEntity(st->ring))
            e->get<PositionComponent>()->setVisible(st->keyboardFocus);

        if (auto e = ecsRef->getEntity(st->radio))
            e->get<ThemeComponent>()->setElement(st->selected ? "radio.mark.on" : "radio.mark");
    }

    void RadioCardSystem::choose(EntityRef face)
    {
        auto st = face->get<RadioCardState>();

        if (st->selected)
            return;

        const _unique_id row = st->cards;
        const std::string tag = st->tag;
        const std::string id = st->id;

        for (auto* other : view<RadioCardState>())
            if (other->cards == row)
                if (auto f = ecsRef->getEntity(other->entityId))
                {
                    other->selected = other->entityId == face.id;
                    applyVisual(f);
                }

        ecsRef->sendEvent(RadioChosenEvent{row, tag, id});
    }

    void RadioCardSystem::onEvent(const HoverChangedEvent& event)
    {
        for (auto id : event.entered)
            if (auto e = ecsRef->getEntity(id); e and e->has<RadioCardState>())
            {
                e->get<RadioCardState>()->hovered = true;
                applyVisual(e);
            }

        for (auto id : event.left)
            if (auto e = ecsRef->getEntity(id); e and e->has<RadioCardState>())
            {
                auto st = e->get<RadioCardState>();
                st->hovered = false;
                st->pressed = false;
                applyVisual(e);
            }
    }

    void RadioCardSystem::onEvent(const OnMouseClick& event)
    {
        if (event.button != 1)
            return;

        for (auto* st : view<RadioCardState>())
            if (st->hovered)
                st->pressed = true;
    }

    void RadioCardSystem::onEvent(const OnMouseRelease& event)
    {
        if (event.button != 1)
            return;

        for (auto* st : view<RadioCardState>())
        {
            if (not st->pressed)
                continue;

            const bool inside = st->hovered;
            st->pressed = false;

            if (inside and not event.cancelled)
                if (auto f = ecsRef->getEntity(st->entityId))
                    choose(f);
        }
    }

    void RadioCardSystem::onEvent(const OnSDLScanCode& event)
    {
        auto* fo = ecsRef->getSystem<FocusOrderSystem>();

        if (not fo or not fo->keyboardFocus())
            return;

        auto face = ecsRef->getEntity(fo->current());

        if (not face or not face->has<RadioCardState>())
            return;

        if (event.key == SDL_SCANCODE_RETURN or event.key == SDL_SCANCODE_SPACE)
        {
            choose(face);
            return;
        }

        int step = 0;

        if (event.key == SDL_SCANCODE_RIGHT or event.key == SDL_SCANCODE_DOWN)
            step = 1;
        else if (event.key == SDL_SCANCODE_LEFT or event.key == SDL_SCANCODE_UP)
            step = -1;

        if (step == 0)
            return;

        auto st = face->get<RadioCardState>();
        const _unique_id row = st->cards;
        const int target = st->index + step;

        // As a group of radios does: the arrows move to the next card and choose it
        for (auto* other : view<RadioCardState>())
            if (other->cards == row and other->index == target)
                if (auto f = ecsRef->getEntity(other->entityId))
                {
                    fo->focus(other->entityId);
                    choose(f);
                }
    }

    void RadioCardSystem::onEvent(const KeyboardFocusChangedEvent& event)
    {
        for (auto* st : view<RadioCardState>())
        {
            auto face = ecsRef->getEntity(st->entityId);

            if (not face)
                continue;

            const bool focused = (st->entityId == event.face) and event.keyboard;

            if (focused != st->keyboardFocus)
            {
                st->keyboardFocus = focused;
                applyVisual(face);
            }
        }
    }
}
