#pragma once

#include "ECS/entitysystem.h"

#include "Scene/scenemanager.h"

namespace pg
{
    class UiComponent;

namespace editor
{
    struct ComponentTab : System<Ref<SceneElement>, InitSys, StoragePolicy>
    {
        ComponentTab();
        ~ComponentTab();

        virtual void init() override;

        void hide();

        EntityRef parent;
        CompRef<UiComponent> parentUi;

        CompRef<UiComponent> backgroundC;
    };
}
}