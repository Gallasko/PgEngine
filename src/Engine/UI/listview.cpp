#include "listview.h"

#include "scrollable.h"

namespace pg
{

    void ListViewSystem::init()
    {

    }

    CompList<UiComponent, ListView> makeListView(EntitySystem *ecs, float x, float y, float width, float height)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        auto scrollable = ecs->attach<Scrollable>(entity);

        auto view = ecs->attach<ListView>(entity);

        scrollable->horizontalSlider = ecs->createEntity();

        scrollable->horizontalCursor = ecs->createEntity();

        scrollable->verticalSlider = ecs->createEntity();

        scrollable->verticalCursor = ecs->createEntity();

        // make2

        ui->setX(x);
        ui->setY(y);

        // Todo fix this in ui component so it does work, right now if the entity is created during runtime the link is not made correctly
        ui->setWidth(width);
        ui->setHeight(height);

        return CompList<UiComponent, ListView>(entity, ui, view);
    }
}