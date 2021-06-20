#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include "../ECS/entitysystem.h"
#include "tileloader.h"
#include "noise.h"
#include "../constant.h"

class Map : private QOpenGLFunctions
{
public:
    enum class ZoneType
    {
        NONE,
        OUTSKIRT,
        TOWNCENTER,
        SHOPCENTER,
        OFFCITY,
        RIVER,
        MOUNTAIN,
        FOREST,
        OTHER
    };

    struct Tiles
    {
        int x;
        int y;
        TilesLoader::TilesId *tileId;
        ZoneType zoneType;

        double nValue;

        Tiles(int x, int y, TilesLoader::TilesId *tileId = nullptr) : x(x), y(y), tileId(tileId) {}
    };

    struct MapConstraint
    {
        unsigned int seed = 0;

        unsigned int width = 10;
        unsigned int height = 10;

        unsigned int lengthLowRoad = 3;

        unsigned int nbOutskirt = 2;
        unsigned int townCenterSize = 5*5;
        unsigned int shopCenterSize = 3*3;

        unsigned int nbOffCity = 0;
        unsigned int offCitySize = 0;

        unsigned int nbRiver = 0;
        unsigned int nbMountain = 1;
        unsigned int nbForest = 1;

        unsigned int obstacleMaxSize = 3*3;
        unsigned int obstacleMinSize = 2*2;

        NoiseParameters noiseParam = {3, 5, 50, -1, 0.70};
    };

    Map(EntitySystem *ecs, TilesLoader *tilesLoader, Map::MapConstraint constraint);
    ~Map();

    void generateMesh();

    inline Map::Tiles*** getTileMap() const { return tileMap; }
    inline unsigned int getWidth() const { return constraint.width; }
    inline unsigned int getHeight() const { return constraint.height; }
    inline QOpenGLVertexArrayObject* getMesh() { if(!meshUpdate) generateMesh(); return VAO; }

private:
    void updateModelInfo();

    EntitySystem *ecs;
    TilesLoader *tilesLoader; 
    Map::MapConstraint constraint;
    NoiseGenerator *noiseGenerator;

    Map::Tiles ***tileMap;

    constant::ModelInfo modelInfo;

    QOpenGLVertexArrayObject *VAO;
    QOpenGLBuffer *VBO;
    QOpenGLBuffer *EBO;

    bool meshUpdate = false;
};
