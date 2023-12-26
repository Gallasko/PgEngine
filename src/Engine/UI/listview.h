#pragma once

#include "ECS/system.h"

#include "uisystem.h"

namespace pg
{

    struct ListView
    {
        EntityRef slider;

        std::vector<EntityRef> entities;
    };

    struct ListViewSystem : public System<Own<ListView>, NamedSystem, InitSys, StoragePolicy>
    {
        virtual std::string getSystemName() const override { return "ListView System"; }

        virtual void init() override;

        void addChild(EntityRef list, EntityRef child);
        
        void removeChild(EntityRef list, EntityRef child);
    };

    /** Helper that create an entity with an Ui component and a Texture component */
    CompList<UiComponent, ListView> makeListView(EntitySystem *ecs, float x, float y, float width, float height);

}