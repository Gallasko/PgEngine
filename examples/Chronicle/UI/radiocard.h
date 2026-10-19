#pragma once

#include <string>
#include <vector>

#include "ECS/entitysystem.h"
#include "Input/inputcomponent.h"   // HoverChangedEvent, OnMouseClick/Release
#include "Input/sdlevents.h"        // OnSDLScanCode
#include "UI/focusable.h"

#include "label.h"
#include "focusorder.h"

namespace chronicle
{
    // One of a few choices, each a card: a radio mark, a small picture of the choice and two lines.
    // The chosen card has its radio filled and a 3 px lapis edge at its left. The cards stand side by
    // side when there is room for them, else one under the other.
    struct RadioCardItem
    {
        std::string id;
        std::string label;                 // control style, ink
        std::string line;                  // body-sm, ink-muted, cut to the card
        std::string swatch;                // A theme id: a 40 px square of that theme's page, ink and red lead, whatever the theme in use. "" for none
    };

    struct RadioCardsSpec
    {
        std::vector<RadioCardItem> items;
        std::string selected;              // An item's id
        float width = 400.0f;              // The row; the cards share it
        std::string tag;                   // carried by RadioChosenEvent
        int z = 20;
    };

    // Sent when the player chooses another card, never when the scene does (select)
    struct RadioChosenEvent { pg::_unique_id cards; std::string tag; std::string id; };

    // On each card's FACE (the hit area). Plain pg::Component; never serialised.
    struct RadioCardState : public pg::Component
    {
        RadioCardState() = default;

        pg::_unique_id cards = 0;          // The row's root
        int index = 0;
        std::string id, tag;
        bool selected = false, hovered = false, pressed = false, keyboardFocus = false;
        pg::_unique_id ground = 0, edge = 0, radio = 0, dot = 0, ring = 0;
    };

    struct RadioCards
    {
        struct Card
        {
            pg::EntityRef face, ground, frame, edge, radio, dot, ring;
            pg::EntityRef swatch;              // The swatch's ground, "" without one
            std::vector<pg::EntityRef> swatchParts;   // Its frame and its three lines
            Label label, line;
        };

        pg::EntityRef root;                // anchor this
        std::vector<Card> cards;
        RadioCardsSpec spec;

        const std::string& selected() const { return spec.selected; }
        void select(pg::EntitySystem*, const std::string& id);   // programmatic: repaints, sends no event
        void setWidth(pg::EntitySystem*, float width);           // The cards share it, side by side or one under the other
        float height() const;
    };

    RadioCards makeRadioCards(pg::EntitySystem*, const RadioCardsSpec&);

    struct RadioCardSystem : public pg::System<pg::Own<RadioCardState>,
        pg::Listener<pg::HoverChangedEvent>, pg::Listener<pg::OnMouseClick>, pg::Listener<pg::OnMouseRelease>,
        pg::Listener<pg::OnSDLScanCode>, pg::Listener<KeyboardFocusChangedEvent>, pg::InitSys>
    {
        std::string getSystemName() const override { return "Chronicle Radio Card System"; }

        void init() override;

        void onEvent(const pg::HoverChangedEvent&) override;
        void onEvent(const pg::OnMouseClick&) override;
        void onEvent(const pg::OnMouseRelease&) override;
        void onEvent(const pg::OnSDLScanCode&) override;   // RETURN/SPACE choose the focused card; the arrows move to the next one and choose it
        void onEvent(const KeyboardFocusChangedEvent&) override;

        void choose(pg::EntityRef face);       // marks it chosen on its row; sends RadioChosenEvent unless it already was
        void applyVisual(pg::EntityRef face);
    };
}
