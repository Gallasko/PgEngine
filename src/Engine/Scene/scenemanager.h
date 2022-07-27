#pragma once

#include "sceneloader.h"

namespace pg
{
    class SceneManager
    {
    public:
        SceneManager();
        ~SceneManager() {}

    private:
        friend void renderer<>(MasterRenderer* masterRenderer, SceneManager* manager);
        
        SceneLoader loader;

        Scene *currentScene = nullptr;
    };
}