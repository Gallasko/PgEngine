#pragma once

#include <string>
#include <vector>

#include "Memory/elementtype.h"
#include "UI/prefabspec.h"   // RecordList

namespace chronicle
{
    // A walk outside the town as it stands: what rules/expedition.pg is handed and hands back,
    // and what the save keeps of a walk under way. The script keeps nothing between two runs.
    struct WalkState
    {
        pg::ElementMap run;            // {zone, mode, ap, hp, seed, ...}: no zone, no walk
        pg::RecordList record;         // Its steps so far: {n, kind, target, count, ap, hp, text, gave, used, figure, tone, glyph, once}
        pg::RecordList marks;          // What each step used, gave, killed and taught: {step, kind, id, n}
        pg::ElementMap fight;          // The fight under way: {mob, turn, breath, stance, over, ...}
        pg::RecordList foes;           // And what he fights: {n, id, name, hp, hpMax, at, alive}
        pg::RecordList account;        // The fight's log: {turn, text, figure, kind}

        // A walk is under way: he is on site, and its screen is the page
        bool under() const
        {
            auto zone = run.find("zone");

            return zone != run.end() and not zone->second.toString().empty();
        }
    };

    // One thing he does on a walk: on site (seek, observe, gather, study, rest, remedy, leave),
    // before an event's card (option), in a fight (pick, target, do, auto, step, back)
    struct WalkAct
    {
        std::string kind;
        std::string target;            // A beast, a thing, an option, a row of the fight's menu
        int count = 0;                 // A pack's size for a seek, the beast aimed at for a target
    };

    // A walk kept: the record of one made by hand, which the same character may walk again
    // without choosing a day or fighting a fight
    struct WalkRecipe
    {
        std::string id;                // "r1", "r2": his, in the order they were walked
        pg::ElementMap fields;         // {zone, name, kind, steps, ap, hp, months, walkedAt, kills, downed}
        pg::RecordList steps;          // The record, frozen
        pg::RecordList marks;          // And its marks
    };

    // The same record so many times, one walk after the other on the life's calendar
    struct WalkQueue
    {
        std::string recipe;            // The record the form is filled for, and the one a queue walks
        int count = 1;                 // How many walks the form asks for
        int left = 0;                  // Walks of a queue still to begin; 0: no queue runs
        int total = 0;                 // How many it began with
        std::string stop = "never";    // When it stops before its last walk: never, bag, item, age
        int value = 0;                 // The amount held, or the age, that stops it
        std::string item;              // The thing a queue of `item` counts
    };
}
