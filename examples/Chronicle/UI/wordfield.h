#pragma once

#include <string>

#include "ECS/entitysystem.h"
#include "Input/inputcomponent.h"   // HoverChangedEvent, OnMouseClick
#include "Input/sdlevents.h"        // OnSDLScanCode, OnSDLTextInput
#include "UI/focusable.h"

#include "label.h"
#include "focusorder.h"

namespace chronicle
{
    // A short word typed in: a 44 px well of vellum-worn in a ruled frame, the word in the control
    // style, a grey placeholder while it is empty. It takes what is typed while it has the focus:
    // clicked, reached with Tab, or given it (focus). Backspace takes the last letter back.
    struct WordFieldSpec
    {
        float width = 240.0f;
        std::string placeholder;
        std::string tag;                   // carried by WordChangedEvent
        size_t most = 24;                  // Letters past it are not taken
        int z = 20;
    };

    // Sent at every letter typed or taken back, never when the scene sets the word (setText)
    struct WordChangedEvent { pg::_unique_id field; std::string tag; std::string text; };

    // On the FACE (the hit area). Plain pg::Component; never serialised.
    struct WordFieldState : public pg::Component
    {
        WordFieldState() = default;

        std::string tag, text;
        size_t most = 24;
        bool hovered = false;
        bool typing = false;               // It has the focus: what is typed goes in
        pg::_unique_id word = 0, placeholder = 0, caret = 0, ring = 0, frame = 0;
    };

    struct WordField
    {
        pg::EntityRef root;                // anchor this; 44 tall
        pg::EntityRef face;                // the hit area: WordFieldState + FocusableComponent
        Label word, placeholder;
        pg::EntityRef ground, frame, caret, ring;
        WordFieldSpec spec;

        std::string text(pg::EntitySystem*) const;
        void setText(pg::EntitySystem*, const std::string&);   // programmatic: sends no event
        void focus(pg::EntitySystem*);                           // It takes what is typed from now
        void setWidth(pg::EntitySystem*, float);
    };

    WordField makeWordField(pg::EntitySystem*, const WordFieldSpec&);

    struct WordFieldSystem : public pg::System<pg::Own<WordFieldState>,
        pg::Listener<pg::HoverChangedEvent>, pg::Listener<pg::OnMouseClick>, pg::Listener<pg::OnSDLTextInput>,
        pg::Listener<pg::OnSDLScanCode>, pg::Listener<KeyboardFocusChangedEvent>, pg::InitSys>
    {
        std::string getSystemName() const override { return "Chronicle Word Field System"; }

        void init() override;

        void onEvent(const pg::HoverChangedEvent&) override;
        void onEvent(const pg::OnMouseClick&) override;          // A click in a field gives it the focus, anywhere else takes it away
        void onEvent(const pg::OnSDLTextInput&) override;
        void onEvent(const pg::OnSDLScanCode&) override;         // BACKSPACE
        void onEvent(const KeyboardFocusChangedEvent&) override;

        // True while a field takes what is typed: the keys are its letters, not the game's shortcuts
        bool anyTyping();

        void setTyping(pg::EntityRef face, bool typing);
        void write(pg::EntityRef face, const std::string& text, bool tell);
        void applyVisual(pg::EntityRef face);
    };
}
