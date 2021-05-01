#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../ECS/entitysystem.h"
#include "tileloader.h"

struct Position
{
    float x;
    float y;
};

struct TileHolder
{
    TilesLoader::Tiles *tileId;
};

class Map
{
public:
    Map(EntitySystem *ecs, TilesLoader *tilesLoader, unsigned int width, unsigned int height, unsigned int nbMaxRoad);

    inline EntitySystem::Entity*** getMap() const { return tileMap; }

private:
    EntitySystem *ecs;
    TilesLoader *tilesLoader;
    unsigned int width, height, nbMaxRoad;

    EntitySystem::Entity*** tileMap;
};
