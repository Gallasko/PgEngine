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
#include "basesystem.h"
#include "../Input/input.h"
#include "../camera.h"

#include <iostream>

// TODO Fix pathfinding sometimes a node is traversed multiple times
// Sometimes the shortest route is not found because a longer route was not registered
// Expend the pathfinding algorithm for open rooms by extending the research kernel
// Add Weight bias to the road
 
class PathFinder 
{
public:
    enum class Dir
    {
        NORTH,
        SOUTH,
        WEST,
        EAST,
        NONE
    };

    struct Road
    {
        unsigned short startNodeId = 0; 
        unsigned short endNodeId = 0;
        unsigned int length = 0;
        PathFinder::Dir dir = PathFinder::Dir::NONE;
    };

    struct Node
    {
        unsigned short id = 0;
        int x, y;
        unsigned short nbAdjNodes;
        PathFinder::Road *path;

        Node(unsigned int id, int x, int y, int nbAdjNodes) : id(id), x(x), y(y), nbAdjNodes(nbAdjNodes) { path = new PathFinder::Road[nbAdjNodes]; }
        Node(const Node& node) : id(node.id), x(node.x), y(node.y), nbAdjNodes(node.nbAdjNodes) { path = new PathFinder::Road[nbAdjNodes]; for(int i = 0; i < nbAdjNodes; i++) path[i] = node.path[i]; }
        ~Node() { delete path; }
    };

    struct Path
    {
        unsigned short startNodeId = 0; 
        unsigned short endNodeId = 0;
        unsigned int length = 0;

        std::vector<PathFinder::Dir> dirList = {};

        //Path() {}
        //Path(const Path& path) : startNodeId(path.startNodeId), endNodeId(path.endNodeId), length(path.length) { dirList.insert(dirList.end(), path.dirList.begin(), path.dirList.end()); }

        Path operator+(const Path& rhs) const
        {
            Path newPath;
            newPath.startNodeId = startNodeId;
            newPath.endNodeId = rhs.endNodeId;
            newPath.length = this->length + rhs.length;
            newPath.dirList.insert(newPath.dirList.end(), dirList.begin(), dirList.end());
            newPath.dirList.insert(newPath.dirList.end(), rhs.dirList.begin(), rhs.dirList.end());

            return newPath;
        }

        Path operator-(const Path& rhs) const
        {
            Path newPath;
            newPath.startNodeId = startNodeId;
            newPath.endNodeId = rhs.endNodeId;
            newPath.length = this->length + rhs.length;

            std::vector<PathFinder::Dir> inverseDirList;

            for(auto dir : dirList)
            {
                //Inverse dir helper func
                switch (dir)
                {
                case PathFinder::Dir::NORTH:
                    inverseDirList.push_back(PathFinder::Dir::SOUTH);
                    break;
                case PathFinder::Dir::SOUTH:
                    inverseDirList.push_back(PathFinder::Dir::NORTH);
                    break;
                case PathFinder::Dir::EAST:
                    inverseDirList.push_back(PathFinder::Dir::WEST);
                    break;
                case PathFinder::Dir::WEST:
                    inverseDirList.push_back(PathFinder::Dir::EAST);
                    break;
                case PathFinder::Dir::NONE:
                    inverseDirList.push_back(PathFinder::Dir::NONE);
                    break;
                }
            }

            newPath.dirList.insert(newPath.dirList.end(), inverseDirList.rbegin(), inverseDirList.rend());
            newPath.dirList.insert(newPath.dirList.end(), rhs.dirList.begin(), rhs.dirList.end());

            return newPath;
        }

        Path& operator+=(const Path& rhs)
        {
            this->length += rhs.length;
            dirList.insert(dirList.end(), rhs.dirList.begin(), rhs.dirList.end());

            return *this;
        }

        //void operator=(const Path& rhs)
        //{
        //    this->startNodeId = rhs.startNodeId;
        //    this->endNodeId = rhs.endNodeId;
        //    this->length = rhs.length;
        //
        //    dirList.insert(dirList.end(), rhs.dirList.begin(), rhs.dirList.end());
        //}

        void operator=(const PathFinder::Road& rhs)
        {
            dirList.clear();

            this->startNodeId = rhs.startNodeId;
            this->endNodeId = rhs.endNodeId;
            this->length = rhs.length;
            
            dirList.insert(dirList.end(), this->length, rhs.dir);
        }

        Path operator-()
        {
            Path newPath;
            newPath.startNodeId = this->startNodeId;
            newPath.endNodeId = this->endNodeId;
            newPath.length = this->length;

            for(auto dir : dirList)
            {
                //Inverse dir helper func
                switch (dir)
                {
                case PathFinder::Dir::NORTH:
                    newPath.dirList.push_back(PathFinder::Dir::SOUTH);
                    break;
                case PathFinder::Dir::SOUTH:
                    newPath.dirList.push_back(PathFinder::Dir::NORTH);
                    break;
                case PathFinder::Dir::EAST:
                    newPath.dirList.push_back(PathFinder::Dir::WEST);
                    break;
                case PathFinder::Dir::WEST:
                    newPath.dirList.push_back(PathFinder::Dir::EAST);
                    break;
                case PathFinder::Dir::NONE:
                    newPath.dirList.push_back(PathFinder::Dir::NONE);
                    break;
                }
            }

            return newPath;
        }
    };

    PathFinder() {}
    ~PathFinder();

    void processMap(float **map, const int& width, const int& height);
    PathFinder::Path getPath(const constant::Vector2D& from, const constant::Vector2D& to);

private:
    std::vector<PathFinder::Node* > findCoreNodes(float **array, const int& width, const int& height);
    void generateAllPath(std::vector<PathFinder::Node* > *nodes);
    PathFinder::Path makePath(const unsigned int& startingNode, const unsigned int& endNode);

    float **map;
    int width = 0;
    int height = 0;

    unsigned int nbNodes = 0;
    std::vector<PathFinder::Node* > nodeList;
    PathFinder::Node ***nodeArray;
    PathFinder::Road ***roadArray;
    PathFinder::Path *currentPath;

    bool initialized = false;
};

/*
template<typename Value>
struct Base : public Variant
{
    Value value;

    Base(Value value) : value(value) {}

    Value getValue() { return value; } 
};
*/

/*
int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setSwapInterval(0);
    //format.setRenderableType(QSurfaceFormat::OpenGL);
    //format.setProfile(QSurfaceFormat::CoreProfile);
    //format.setVersion(3, 3);

    QSurfaceFormat::setDefaultFormat(format);
	QGuiApplication app(argc, argv);

	//GameWindow game;
    //game.resize(640, 480);
    //game.setAnimating(true);
    //game.show();
    //
    //QObject::connect(&game, SIGNAL(quitApp()), &app, SLOT(quit()));

    //auto number = new Base<double>(5.5f);
    //Variant* val = number;

    //std::cout << (*val).getValue() << std::endl;

    // [Create Array]

    //float array[10][10] = { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    //                        0, 1, 1, 0, 0, 1, 0, 0, 0, 0,
    //                        0, 0, 1, 0, 0, 1, 0, 0, 0, 0,
    //                        0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
    //                        0, 0, 1, 0, 1, 0, 0, 1, 0, 0,
    //                        0, 0, 1, 0, 1, 0, 0, 1, 1, 1,
    //                        0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
    //                        0, 1, 1, 0, 1, 0, 0, 1, 1, 0,
    //                        0, 0, 0, 0, 1, 0, 0, 0, 1, 0,
    //                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    float array[10][10] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, 0,
                            1, 1, 1, 0, 0, 1, 0, 0, 1, 1,
                            0, 0, 1, 0, 0, 1, 1, 1, 1, 0,
                            1, 1, 1, 1, 1, 1, 0, 1, 0, 0,
                            0, 0, 1, 0, 1, 0, 0, 1, 0, 0,
                            0, 0, 1, 0, 1, 0, 0, 1, 1, 1,
                            0, 0, 1, 1, 1, 1, 1, 1, 0, 1,
                            0, 1, 1, 0, 1, 0, 0, 1, 1, 1,
                            1, 1, 0, 0, 1, 1, 0, 0, 0, 1,
                            0, 1, 0, 0, 0, 1, 1, 0, 0, 1 };

    // [Print Map]

    for(int i = 0; i < 10; i++)
    {
        for(int j = 0; j < 10; j++)
        {
                //std::cout << "\x1B[31m" << array[i][j] << "\033[0m ";

            if(array[i][j] == 1)
                std::cout << array[i][j] << " ";
            else
                std::cout << array[i][j] << " ";

        }
        std::cout << std::endl;
    }
    //std::cout << "\x1B[31m"

    float array2[10][10] = {0};

    auto nodes = findCoreNodes(array, 10, 10, array2);

    for(int i = 0; i < 10; i++)
    {
        for(int j = 0; j < 10; j++)
        {
                //std::cout << "\x1B[31m" << array[i][j] << "\033[0m ";

            if(array2[i][j] == 1)
                std::cout << array2[i][j] - 1 << " ";
            else
                std::cout << array2[i][j] - 1 << " ";

        }
        std::cout << std::endl;
    }

    std::cout << "Nb nodes found: " << nodes.size() << std::endl;

    for(auto node : nodes)
    {
        std::cout << "Nodes in " << node->x << ", " << node->y;
        
        for(int i = 0; i < node->nbAdjNodes; i++)
        {
            switch (node->path[i].dir)
            {
            case Dir::NORTH:
                std::cout << " NORTH Path length: ";
                break;
            case Dir::SOUTH:
                std::cout << " SOUTH Path length: ";
                break;
            case Dir::EAST:
                std::cout << " EAST Path length: ";
                break;
            case Dir::WEST:
                std::cout << " WEST Path length: ";
                break;
            }

            std::cout << node->path[i].length;
        }
        
        std::cout << std::endl;
    }

    generateAllPath(&nodes);

    std::cout << "end" << std::endl;

	//return app.exec();
    return 0;
}
*/

class Map : private QOpenGLFunctions, public Base
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
    
        operator float() { return tileId == nullptr ? 0.0f : *tileId == TileType::ROAD ? 1.0f : 0.0f; } 
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

    void changeTile(Input*, double deltaTime...) { va_list args; va_start(args, deltaTime); tileToBePlaced = va_arg(args, TilesLoader::TilesId*); }
    
    void runPathFinding(Input* inputHandler, double deltaTime...);
    
    void clicked(Input* inputHandler, double deltaTime...); // expect as argument : Input*, double, int, int, double, Camera*;

    void switchToPathFind(Input* inputHandler, double deltaTime...) { static bool switched = false; if(inputHandler->isKeyPressed(Qt::Key_E) && !switched) { pathFindLookUp = !pathFindLookUp; switched = true; } if(inputHandler->isKeyPressed(Qt::Key_R) && !switched) { pathRoad = !pathRoad; switched = true; drawPath(); } if(!inputHandler->isKeyPressed(Qt::Key_E) && !inputHandler->isKeyPressed(Qt::Key_R)) switched = false; }

    //void changeTile(Input* inputHandler, double deltaTime, unsigned int tile) {std::cout << tile << std::endl; }

private:
    void updateModelInfo();
    void roadTiling();
    void drawPath();

    EntitySystem *ecs;
    TilesLoader *tilesLoader; 
    Map::MapConstraint constraint;
    NoiseGenerator *noiseGenerator;

    Map::Tiles ***tileMap;

    constant::ModelInfo modelInfo;

    QOpenGLVertexArrayObject *VAO;
    QOpenGLBuffer *VBO;
    QOpenGLBuffer *EBO;

    PathFinder pathFinder;

    TilesLoader::TilesId *tileToBePlaced = nullptr;
    bool pathFindingInitialised = false;

    bool meshUpdate = false;

    bool pathFindLookUp = false;

    constant::Vector2D startPath;
    bool pathRoad = false;

    PathFinder::Path pathToRender;
};
