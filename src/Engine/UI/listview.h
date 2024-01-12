#pragma once

#include "ECS/system.h"

#include "uisystem.h"

namespace pg
{

    struct ListView
    {
        
    };

    struct ListViewSystem : public System<Own<ListView>, NamedSystem, InitSys, StoragePolicy>
    {
        virtual std::string getSystemName() const override { return "ListView System"; }

        virtual void init() override;
    };

    /** Helper that create an entity with an Ui component and a Texture component */
    CompList<UiComponent, ListView> makeListView(EntitySystem *ecs, float x, float y, float width, float height);

}