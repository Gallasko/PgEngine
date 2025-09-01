#pragma once

#include "engine.h"

class GameApp {
private:
    pg::Engine engine;

public:
    GameApp(const std::string& appName);
    int exec();
};