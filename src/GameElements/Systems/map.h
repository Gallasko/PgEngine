#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <queue>
#include <mutex>

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include "../../Engine/ECS/entitysystem.h"
#include "../../Engine/Loaders/tileloader.h"
#include "../../Engine/Maths/noise.h"
#include "../../Engine/constant.h"
#include "../../Engine/Input/input.h"
#include "../../Engine/camera.h"

// TODO Fix pathfinding sometimes a node is traversed multiple times
// Sometimes the shortest route is not found because a longer route was not registered
// Expend the pathfinding algorithm for open rooms by extending the research kernel
// Add Weight bias to the road

namespace std
{
    template <>
    struct hash<pg::constant::Vector2D> {
        size_t operator()(const pg::constant::Vector2D& k) const {
            return ((hash<int>()(k.x) ^ (hash<int>()(k.y) << 1)) >> 1);
        }
    };
}

namespace pg
{
    struct MapFloat
    {
        const float **map;
        unsigned int width, height;
    };

    class Path2D
    {
    public:
        Path2D() {}
        Path2D(const MapFloat& map) : map(map) {}

        inline void setMap(const MapFloat& map) { this->map = map; }

        std::vector<constant::Vector2D> operator()(const constant::Vector2D& from, const constant::Vector2D& to) const
        {
            std::unordered_map<constant::Vector2D, float> dist;
            std::unordered_map<constant::Vector2D, constant::Vector2D> prev;

            prev[from] = constant::Vector2D{std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
            prev[to] = constant::Vector2D{std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};

            struct queue_node
            {
                constant::Vector2D value;
                float dist;
            };

            auto cmp = [&](const queue_node& a, const queue_node& b){
                return a.dist > b.dist;
            };

            std::priority_queue<queue_node, std::vector<queue_node>, decltype(cmp)> Q(cmp);

            for (unsigned int x = 0; x < map.width; x++) 
            {
                for (unsigned int y = 0; y < map.height; y++) 
                {
                    const constant::Vector2D coord = {x, y};

                    if(getMapDistance(coord) > 0.0f) // if map value = 0 it means that the cell is not reachable 
                    {
                        dist[coord] = std::numeric_limits<float>::max();
                        Q.push({coord, std::numeric_limits<float>::max()});

                        prev[coord] = constant::Vector2D{std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
                    }
                }
            }

            dist[from] = 0;
            dist[to] = std::numeric_limits<float>::max();
            Q.push({from, 0});

            // Search loop
            while (!Q.empty()) 
            {
                auto u = Q.top();
                Q.pop();

                // Old priority queue value
                if (u.dist != dist[u.value])
                    continue;

                if (u.value == to)
                    break;

                for (const constant::Vector2D& v : getNeighbours(u.value)) 
                {
                    const float alt = dist[u.value] + getMapDistance(v);
                    if (alt < dist[v]) 
                    {
                        dist[v] = alt;
                        Q.push({v, alt});

                        prev[v] = u.value;
                    }
                }
            }

            // Trace path - if there is one
            std::vector<constant::Vector2D> path;

            if (prev[to].x != std::numeric_limits<float>::min()) // Todo: right now the condition is on -1 but the map could go to the negativ so i need to keep that in mind and change the end condition
            {
                constant::Vector2D current = to;

                while (current.x != std::numeric_limits<float>::min()) 
                {
                    path.push_back(current);
                    current = prev[current];
                }

                std::reverse(path.begin(), path.end());
            }

            return path;
        }

    private:
        std::vector<constant::Vector2D> getNeighbours(const constant::Vector2D& place) const
        {
            std::vector<constant::Vector2D> neighbours;

            for (int dx = -1; dx <= 1; dx++) 
            {
                for (int dy = -1; dy <= 1; dy++) 
                {
                    const auto coord = place + constant::Vector2D{dx, dy};

                    const bool notSelf = !(dx == 0 && dy == 0);
                    const bool connectivity = abs(dx) + abs(dy) <= 1; // with one we don t take diagonal points, otherwise with two we take diagonal points
                    const bool withinGrid = coord.x >= 0 && coord.y >= 0 && coord.x < map.width && coord.y < map.height;

                    if (notSelf && connectivity && withinGrid) {
                        neighbours.push_back(coord);
                    }
                }
            }

            return neighbours;
        }

        float getMapDistance(const constant::Vector2D& pos) const
        {
            return pos.x >= 0 && pos.y >= 0 && pos.x < map.width && pos.y < map.height ? map.map[static_cast<int>(pos.x)][static_cast<int>(pos.y)] : std::numeric_limits<float>::max();
        }

        MapFloat map = {nullptr, 0, 0};
    };

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
            const TilesLoader::TilesId *tileId;
            ZoneType zoneType;

            double nValue;

            Tiles(int x, int y, const TilesLoader::TilesId *tileId = nullptr) : x(x), y(y), tileId(tileId) {}
        
            //operator float() { return tileId == nullptr ? 0.0f : *tileId == TileType::ROAD ? 1.0f : *tileId == TileType::HOUSE ? 5.0f : 0.0f; } 
            operator float() const { return tileId == nullptr ? 0.0f : *tileId == TileType::ROAD ? 1.0f : 0.0f; } 
        };

        struct MapConstraint
        {
            unsigned int seed = 0;

            unsigned int width = 10;
            unsigned int height = 10;

            unsigned int lengthLowRoad = 3;

            unsigned int nbOutskirt = 2;
            unsigned int townCenterSize = 5 * 5;
            unsigned int shopCenterSize = 3 * 3;

            unsigned int nbOffCity = 0;
            unsigned int offCitySize = 0;

            unsigned int nbRiver = 0;
            unsigned int nbMountain = 1;
            unsigned int nbForest = 1;

            unsigned int obstacleMaxSize = 3 * 3;
            unsigned int obstacleMinSize = 2 * 2;

            NoiseParameters noiseParam = {3, 5, 50, -1, 0.70};
        };

        Map(EntitySystem *ecs, TilesLoader *tilesLoader, const Map::MapConstraint& constraint);
        ~Map();

        void generateMesh();

        inline Map::Tiles*** getTileMap() const { return tileMap; }
        inline unsigned int getWidth() const { return constraint.width; }
        inline unsigned int getHeight() const { return constraint.height; }
        inline QOpenGLVertexArrayObject* getMesh() { if(!meshUpdate) generateMesh(); return VAO; }

        void changeTile(Input* inputHandler, double deltaTime...) { va_list args; va_start(args, deltaTime); if(inputHandler->isButtonPressed(Qt::LeftButton)) tileToBePlaced = va_arg(args, TilesLoader::TilesId*); }
        
        void runPathFinding(Input* inputHandler, double deltaTime...);
        
        void clicked(Input* inputHandler, double deltaTime...); // expect as argument : Input*, double, UiSize*, UiSize*, double*, Camera*;

        void switchToPathFind(Input* inputHandler, double...) { static bool switched = false; if(inputHandler->isKeyPressed(Qt::Key_E) && !switched) { pathFindLookUp = !pathFindLookUp; switched = true; } if(inputHandler->isKeyPressed(Qt::Key_R) && !switched) { pathRoad = !pathRoad; switched = true; drawPath(); } if(!inputHandler->isKeyPressed(Qt::Key_E) && !inputHandler->isKeyPressed(Qt::Key_R)) switched = false; }

        std::vector<constant::Vector2D> createPathBetweenHouseAndShop();

    private:
        void initPathFinding();
        void updateModelInfo();
        void generateRandomMap();
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

        Path2D pathFinder;
        bool floatMapInitialised = false;
        const float **floatMap;

        std::vector<constant::Vector2D> housePos;
        std::vector<constant::Vector2D> shopPos;

        const TilesLoader::TilesId *tileToBePlaced = nullptr;
        bool pathFindingInitialised = false;

        bool meshUpdate = false;

        bool pathFindLookUp = false;

        constant::Vector2D startPath;
        bool pathRoad = false;

        std::mutex syncMutex;
    };
}