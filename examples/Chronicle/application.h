#ifndef CHRONICLE_APPLICATION_H
#define CHRONICLE_APPLICATION_H

#include <string>
#include <utility>
#include <vector>

#include "engine.h"

namespace chronicle
{
    struct LaunchOptions
    {
        std::string devScene;          // "" = the Life scene
        std::string theme;             // --theme day|candle: this session only. "" takes the one the player chose (kept in the save's file, savePath)
        bool fresh = false;            // --fresh: a first life, a month before 7, led by its guide
        int guide = 0;                 // --guide N, with --fresh: the guide past its step N
        float age = 0.0f;              // --age N, with --fresh: opened at that age (7 or more), his sixth year not played
        bool noSave = false;           // --no-save: the mockup's life, never written
        bool freshWithoutSave = false; // No save to load: a first life, not the mockup's (the web build)
        std::string savePath = "save/chronicle/life.sz";
        float monthMs = 2000.0f;       // --month-ms N: one month every N ms while the months run
        std::string walk;              // --walk <zone>: a life that is never written, opened on a walk to that zone (hedgerows, millpond, hollow, quarry)
        int walkSeed = 0;              // --walk-seed N: the dice of the walks, so that a walk can be tried again as it went; 0 takes them from the clock
        std::string build;             // --build <id>: a life of one of the builds to try fights with (rules/testbuilds.pg), every place known; "list" says them
        std::vector<std::pair<std::string, int>> sets;   // --set <stat>=<n> (repeated): a figure set on top of the build
        bool saveAsked = false;        // --save was given: a life of --walk or --build is written there
        int width = 1320;              // --size WxH: the window at launch
        int height = 1020;
    };

    class GameApp
    {
    public:
        GameApp(const std::string& appName, const LaunchOptions& options);
        ~GameApp();

        int exec();

    private:
        pg::Engine engine;
        LaunchOptions opt;
    };
}

#endif
