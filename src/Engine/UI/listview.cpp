#include "listview.h"

namespace pg
{

    void ListViewSystem::init()
    {

    }

    void ListViewSystem::addChild(EntityRef list, EntityRef child)
    {

    }
    
    void ListViewSystem::removeChild(EntityRef list, EntityRef child)
    {

    }

    CompList<UiComponent, ListView> makeListView(EntitySystem *ecs, float x, float y, float width, float height)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        auto view = ecs->attach<ListView>(entity);

        auto sliderEntity = ecs->createEntity();

        // make2

        ui->setX(x);
        ui->setY(y);

        // Todo fix this in ui component so it does work, right now if the entity is created during runtime the link is not made correctly
        ui->setWidth(width);
        ui->setHeight(height);

        return CompList<UiComponent, ListView>(entity, ui, view);
    }
}