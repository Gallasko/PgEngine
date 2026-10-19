#pragma once

#include <string>

namespace pg { class Serializer; }

namespace chronicle
{
    // What the player chose on the Options page. Kept in the save's own file, as an object of its
    // own beside the life ("settings" next to "life"), and never part of a life: a save loaded takes
    // only the life out of its file and leaves these as they are, and a wipe writes the file again
    // with these alone. A file without them reads as the defaults; an unknown theme reads as "day".
    struct Settings
    {
        static constexpr int Version = 1;

        // A click on a work of the Life page begins it. Off: the click chooses it, and Begin or a
        // double click begins it (the user's convention of 2026-10-17, which stays the default)
        bool beginOnFirstClick = false;

        std::string theme = "day";     // "day" (Scriptorium) or "candle" (Candle)

        // False when the file is missing or holds no settings: the defaults are kept
        bool load(const std::string& path);

        // A file of the settings alone (the save's file once the chronicle is burnt); its folder made
        // if need be. Whatever the file held is gone
        bool save(const std::string& path) const;

        // As the "settings" object of a file being written: LifeSave::save writes them beside the life
        void writeInto(pg::Serializer& serializer) const;
    };
}
