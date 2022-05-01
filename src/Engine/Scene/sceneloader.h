#pragma once

namespace pg
{
    class Scene
    {
    public:
        Scene();
        ~Scene();

        void onEnter();
        void onLeave();
    };

    class SceneLoader
    {
    public:
        SceneLoader();
        ~SceneLoader();
    };
}