#pragma once

#include <string>

#include "ECS/entitysystem.h"
#include "Input/inputcomponent.h"   // HoverChangedEvent, OnMouseClick/Release
#include "Input/sdlevents.h"        // OnSDLScanCode
#include "UI/focusable.h"

#include "label.h"
#include "focusorder.h"

namespace chronicle
{
    // An option that is on or off: its word ("On", "Off") and, at its right, a 44 x 24 track whose
    // knob stands at the right when on. On, the track is verdigris. One click, Enter or Space on the
    // focused switch turns it over.
    struct SwitchSpec
    {
        bool on = false;
        std::string onText = "On";
        std::string offText = "Off";
        std::string tag;                   // carried by SwitchToggledEvent
        int z = 20;
    };

    // Sent when the player turns it over, never when the scene sets it (setOn)
    struct SwitchToggledEvent { pg::_unique_id face; std::string tag; bool on; };

    // On the FACE (the hit area). Plain pg::Component; never serialised.
    struct SwitchState : public pg::Component
    {
        SwitchState() = default;

        bool on = false;
        bool hovered = false, pressed = false, keyboardFocus = false;
        std::string tag;
        std::string onText, offText;

        pg::_unique_id track = 0, frame = 0, knob = 0, ring = 0, word = 0;
    };

    struct Switch
    {
        pg::EntityRef root;                // anchor this; 44 tall
        pg::EntityRef face;                // the hit area, the whole switch: SwitchState + FocusableComponent
        Label word;
        pg::EntityRef track, frame, knob, ring;
        SwitchSpec spec;

        bool on(pg::EntitySystem*) const;
        void setOn(pg::EntitySystem*, bool);   // programmatic: repaints, sends no event
    };

    Switch makeSwitch(pg::EntitySystem*, const SwitchSpec&);

    struct SwitchSystem : public pg::System<pg::Own<SwitchState>,
        pg::Listener<pg::HoverChangedEvent>, pg::Listener<pg::OnMouseClick>, pg::Listener<pg::OnMouseRelease>,
        pg::Listener<pg::OnSDLScanCode>, pg::Listener<KeyboardFocusChangedEvent>, pg::InitSys>
    {
        std::string getSystemName() const override { return "Chronicle Switch System"; }

        void init() override;

        void onEvent(const pg::HoverChangedEvent&) override;
        void onEvent(const pg::OnMouseClick&) override;
        void onEvent(const pg::OnMouseRelease&) override;
        void onEvent(const pg::OnSDLScanCode&) override;   // RETURN/SPACE on the focused switch
        void onEvent(const KeyboardFocusChangedEvent&) override;

        void toggle(pg::EntityRef face);       // turns it over and sends SwitchToggledEvent
        void applyVisual(pg::EntityRef face);  // re-keys the parts from state; public for tests
    };
}
