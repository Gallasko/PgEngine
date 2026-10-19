#include "switch.h"

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL.h>
#elif __linux__
#include <SDL2/SDL.h>
#elif _WIN32
#include <SDL.h>
#endif

#include <algorithm>

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
        struct SwitchNoOp {};

        constexpr float FACE_H = 44.0f;    // The least a control is tall
        constexpr float PAD_X = 8.0f;      // space-2 at either end
        constexpr float GAP = 8.0f;        // space-2 between the word and the track
        constexpr float TRACK_W = 44.0f;
        constexpr float TRACK_H = 24.0f;
        constexpr float KNOB = 16.0f;
        constexpr float KNOB_IN = 4.0f;    // From the track's edge: inside its 2 px frame, and 2 px of air
        constexpr float LINE_H = 20.0f;    // The control style's line

        // The theme defines "switch.<part>" at rest (off) and its ".on" leaf
        std::string part(const std::string& name, bool on) { return "switch." + name + (on ? ".on" : ""); }

        float knobMargin(bool on) { return on ? TRACK_W - KNOB_IN - KNOB : KNOB_IN; }
    }

    Switch makeSwitch(EntitySystem* ecs, const SwitchSpec& spec)
    {
        auto* ttf = ecs->getSystem<TTFTextSystem>();
        auto* theme = ecs->getSystem<ThemeSystem>();
        const int z = spec.z;

        Switch s;
        s.spec = spec;

        // The word takes the room of the longer of the two: the track does not move when it turns over
        const TextStyle& control = theme->style("control");
        const float wordW = std::max(ttf->measureText(control.fontAlias, spec.onText, 1.0f, 0.0f, 0.0f, control.letterSpacingPx).width,
                                     ttf->measureText(control.fontAlias, spec.offText, 1.0f, 0.0f, 0.0f, control.letterSpacingPx).width);
        const float faceW = PAD_X + wordW + GAP + TRACK_W + PAD_X;

        auto root = makeAnchoredPrefab(ecs, 0.0f, 0.0f, static_cast<float>(z));
        root.get<PositionComponent>()->setWidth(faceW);
        root.get<PositionComponent>()->setHeight(FACE_H);
        const _unique_id rootId = root.id;
        s.root = root.entity;

        // face: the whole switch
        auto face = ecs->createEntity();
        auto fp = ecs->attach<PositionComponent>(face);
        fp->setWidth(faceW); fp->setHeight(FACE_H); fp->setZ(static_cast<float>(z));
        auto fa = ecs->attach<UiAnchor>(face);
        fa->fillIn(root.get<UiAnchor>());
        fa->setZConstrain(PosConstrain{rootId, AnchorType::Z});
        ecs->attach<FocusableComponent>(face);
        auto st = ecs->attach<SwitchState>(face);
        st->on = spec.on; st->tag = spec.tag; st->onText = spec.onText; st->offText = spec.offText;
        const _unique_id faceId = face.id;
        root.get<Prefab>()->addToPrefab(face, "face");
        s.face = face;

        if (auto* fo = ecs->getSystem<FocusOrderSystem>())
            fo->add(faceId);

        // the word
        LabelSpec ls; ls.style = "control"; ls.text = spec.on ? spec.onText : spec.offText; ls.color = spec.on ? "verdigris" : "ink-muted"; ls.z = z + 3;
        s.word = makeLabel(ecs, ls);
        auto wa = s.word.entity->get<UiAnchor>();
        wa->setLeftAnchor(PosAnchor{faceId, AnchorType::Left}); wa->setLeftMargin(PAD_X);
        wa->setTopAnchor(PosAnchor{faceId, AnchorType::Top}); wa->setTopMargin((FACE_H - LINE_H) / 2.0f);
        wa->setZConstrain(PosConstrain{faceId, AnchorType::Z, PosOpType::Add, 3.0f});
        root.get<Prefab>()->addToPrefab(s.word.entity);
        st->word = s.word.entity.id;

        // the track, its frame and the knob
        const float trackX = PAD_X + wordW + GAP;
        const float trackY = (FACE_H - TRACK_H) / 2.0f;

        auto track = makeRoundedRect2DShape(ecs, theme->radius("radius-md"), TRACK_W, TRACK_H);
        auto ta = track.get<UiAnchor>();
        ta->setLeftAnchor(PosAnchor{faceId, AnchorType::Left}); ta->setLeftMargin(trackX);
        ta->setTopAnchor(PosAnchor{faceId, AnchorType::Top}); ta->setTopMargin(trackY);
        ta->setZConstrain(PosConstrain{faceId, AnchorType::Z, PosOpType::Add, 1.0f});
        ecs->attach<ThemeComponent>(track.entity, part("track", spec.on));
        root.get<Prefab>()->addToPrefab(track.entity);
        s.track = track.entity;
        st->track = track.entity.id;

        auto frame = makeStrokeRect2DShape(ecs, TRACK_W, TRACK_H, {255.0f, 255.0f, 255.0f, 255.0f}, 2.0f);
        auto ra = frame.get<UiAnchor>();
        ra->setLeftAnchor(PosAnchor{faceId, AnchorType::Left}); ra->setLeftMargin(trackX);
        ra->setTopAnchor(PosAnchor{faceId, AnchorType::Top}); ra->setTopMargin(trackY);
        ra->setZConstrain(PosConstrain{faceId, AnchorType::Z, PosOpType::Add, 2.0f});
        ecs->attach<ThemeComponent>(frame.entity, part("frame", spec.on));
        root.get<Prefab>()->addToPrefab(frame.entity);
        s.frame = frame.entity;
        st->frame = frame.entity.id;

        auto knob = makeRoundedRect2DShape(ecs, 2.0f, KNOB, KNOB);
        auto ka = knob.get<UiAnchor>();
        ka->setLeftAnchor(PosAnchor{track.entity.id, AnchorType::Left}); ka->setLeftMargin(knobMargin(spec.on));
        ka->setTopAnchor(PosAnchor{track.entity.id, AnchorType::Top}); ka->setTopMargin((TRACK_H - KNOB) / 2.0f);
        ka->setZConstrain(PosConstrain{faceId, AnchorType::Z, PosOpType::Add, 3.0f});
        ecs->attach<ThemeComponent>(knob.entity, part("knob", spec.on));
        root.get<Prefab>()->addToPrefab(knob.entity);
        s.knob = knob.entity;
        st->knob = knob.entity.id;

        // ring: 2 px focus-ink, 4 px outside the face
        auto ring = makeStrokeRect2DShape(ecs, 1.0f, 1.0f, {255.0f, 255.0f, 255.0f, 255.0f}, theme->border("border-rule"));
        auto ga = ring.get<UiAnchor>();
        ga->setLeftAnchor(PosAnchor{faceId, AnchorType::Left});     ga->setLeftMargin(-4.0f);
        ga->setRightAnchor(PosAnchor{faceId, AnchorType::Right});   ga->setRightMargin(-4.0f);
        ga->setTopAnchor(PosAnchor{faceId, AnchorType::Top});       ga->setTopMargin(-4.0f);
        ga->setBottomAnchor(PosAnchor{faceId, AnchorType::Bottom}); ga->setBottomMargin(-4.0f);
        ga->setZConstrain(PosConstrain{faceId, AnchorType::Z, PosOpType::Add, 4.0f});
        ring.get<PositionComponent>()->setVisible(false);
        ecs->attach<ThemeComponent>(ring.entity, "switch.ring");
        root.get<Prefab>()->addToPrefab(ring.entity);
        s.ring = ring.entity;
        st->ring = ring.entity.id;

        return s;
    }

    bool Switch::on(EntitySystem*) const
    {
        return not face.empty() and face.get<SwitchState>()->on;
    }

    void Switch::setOn(EntitySystem* ecs, bool value)
    {
        spec.on = value;

        if (face.empty())
            return;

        face->get<SwitchState>()->on = value;

        if (auto* sys = ecs->getSystem<SwitchSystem>())
            sys->applyVisual(face);
    }

    // ── SwitchSystem ───────────────────────────────────────────────────────────
    void SwitchSystem::init()
    {
        auto group = registerGroup<SwitchState>();
        group->addOnGroup([this](EntityRef entity) {
            if (not entity->has<MouseEnterComponent>())
                ecsRef->attach<MouseEnterComponent>(entity, makeCallable<SwitchNoOp>(SwitchNoOp{}));
            if (not entity->has<MouseLeaveComponent>())
                ecsRef->attach<MouseLeaveComponent>(entity, makeCallable<SwitchNoOp>(SwitchNoOp{}));
        });
    }

    void SwitchSystem::applyVisual(EntityRef face)
    {
        auto st = face->get<SwitchState>();

        auto rekey = [this](_unique_id id, const std::string& element) {
            if (auto e = ecsRef->getEntity(id))
                e->get<ThemeComponent>()->setElement(element);
        };

        rekey(st->track, part("track", st->on));
        rekey(st->frame, part("frame", st->on));
        rekey(st->knob, part("knob", st->on));

        if (auto knob = ecsRef->getEntity(st->knob))
            knob->get<UiAnchor>()->setLeftMargin(knobMargin(st->on));

        // The word is a bare label entity: its text and its element, directly. It is anchored at its
        // left, so its width is no matter
        if (auto word = ecsRef->getEntity(st->word))
        {
            word->get<TTFText>()->setText(st->on ? st->onText : st->offText);
            word->get<ThemeComponent>()->setElement(labelElement("control", st->on ? "verdigris" : (st->hovered ? "ink" : "ink-muted")));
        }

        if (auto ring = ecsRef->getEntity(st->ring))
            ring->get<PositionComponent>()->setVisible(st->keyboardFocus);
    }

    void SwitchSystem::toggle(EntityRef face)
    {
        auto st = face->get<SwitchState>();

        st->on = not st->on;
        applyVisual(face);

        ecsRef->sendEvent(SwitchToggledEvent{face.id, st->tag, st->on});
    }

    void SwitchSystem::onEvent(const HoverChangedEvent& event)
    {
        for (auto id : event.entered)
            if (auto e = ecsRef->getEntity(id); e and e->has<SwitchState>())
            {
                e->get<SwitchState>()->hovered = true;
                applyVisual(e);
            }

        for (auto id : event.left)
            if (auto e = ecsRef->getEntity(id); e and e->has<SwitchState>())
            {
                auto st = e->get<SwitchState>();
                st->hovered = false;
                st->pressed = false;
                applyVisual(e);
            }
    }

    void SwitchSystem::onEvent(const OnMouseClick& event)
    {
        if (event.button != 1)
            return;

        for (auto* st : view<SwitchState>())
            if (st->hovered)
                st->pressed = true;
    }

    void SwitchSystem::onEvent(const OnMouseRelease& event)
    {
        if (event.button != 1)
            return;

        for (auto* st : view<SwitchState>())
        {
            if (not st->pressed)
                continue;

            const bool inside = st->hovered;
            st->pressed = false;

            // A cancelled release (the press scrolled the page) is not a click
            if (inside and not event.cancelled)
                if (auto f = ecsRef->getEntity(st->entityId))
                    toggle(f);
        }
    }

    void SwitchSystem::onEvent(const OnSDLScanCode& event)
    {
        if (event.key != SDL_SCANCODE_RETURN and event.key != SDL_SCANCODE_SPACE)
            return;

        auto* fo = ecsRef->getSystem<FocusOrderSystem>();

        // Only the switch the keyboard is on (its ring showing), not the last one clicked
        if (not fo or not fo->keyboardFocus())
            return;

        auto face = ecsRef->getEntity(fo->current());

        if (face and face->has<SwitchState>())
            toggle(face);
    }

    void SwitchSystem::onEvent(const KeyboardFocusChangedEvent& event)
    {
        for (auto* st : view<SwitchState>())
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
