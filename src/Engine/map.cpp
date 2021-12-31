#include "map.h"

namespace pg
{
    Map::Map(EntitySystem *ecs, TilesLoader *tilesLoader, Map::MapConstraint constraint) : ecs(ecs), tilesLoader(tilesLoader), constraint(constraint)
    {
        initializeOpenGLFunctions(); 

        VAO = new QOpenGLVertexArrayObject();
        VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

        VAO->create();
        VBO->create();
        EBO->create();

        tileMap = new Map::Tiles**[constraint.width];

        for(unsigned int i = 0; i < constraint.width; i++)
        {
            tileMap[i] = new Map::Tiles*[constraint.height];

            for(unsigned int j = 0; j < constraint.height; j++)
                tileMap[i][j] = new Tiles(i, j, tilesLoader->getTile("Dirt"));
        }

        noiseGenerator = new NoiseGenerator(constraint.seed);
        noiseGenerator->setParameters(constraint.noiseParam);

        srand(constraint.seed);

        for(unsigned int i = 0; i < constraint.width; i++)
        {
            for(unsigned int j = 0; j < constraint.height; j++)
            {
                auto nValue = noiseGenerator->noise2D(i, j);
                
                tileMap[i][j]->nValue = nValue;

                if(nValue <= 5.5 && nValue > 3.5)
                    tileMap[i][j]->tileId = tilesLoader->getTile("Grass");
                else if(nValue > 7.0)
                    tileMap[i][j]->tileId = tilesLoader->getTile("Mountain");
                else if(nValue <= 3.5)
                    tileMap[i][j]->tileId = tilesLoader->getTile("Water");

            } 
        }

        meshUpdate = false;

        tileToBePlaced = tilesLoader->getTile("Dirt");
    }

    Map::~Map()
    {
        for(int i = constraint.width - 1; i >= 0; i--)
        {
            for(int j = constraint.height - 1; j >= 0; j--)
            {
                delete tileMap[i][j];
            }

            delete[] tileMap[i];
        }

        delete[] tileMap;

        delete noiseGenerator;

        delete VAO;
        delete VBO;
        delete EBO;
    }

    void Map::generateMesh()
    {
        updateModelInfo();

        VAO->bind();

        // position attribute
        VBO->bind();
        VBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
        VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        // texture coord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        EBO->bind();
        EBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
        EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        VAO->release();

        meshUpdate = true;
    }

    void Map::runPathFinding(Input* inputHandler, double...)
    {
        if(inputHandler->isButtonPressed(Qt::LeftButton))
            initPathFinding();
    }

    void Map::clicked(Input* inputHandler, double deltaTime...)
    {
        static bool clickedOnce = false;

        if(inputHandler->isButtonPressed(Qt::LeftButton))
        {
            auto mousePos = inputHandler->getMousePos();

            va_list args; 
            va_start(args, deltaTime); 
            
            auto screenWidth = va_arg(args, int);
            auto screenHeight = va_arg(args, int);
            auto gameScale = va_arg(args, double);
            auto camera = va_arg(args, Camera*);

            va_end(args);

            float selectedTileX = ((float)(mousePos.x() - screenWidth / 2.0f )) / (gameScale / 2.0f) + camera->Position.x() * screenWidth / gameScale;
            float selectedTileY = ((float)(screenHeight / 2.0f - mousePos.y())) / (gameScale / 4.0f) + camera->Position.y() * screenHeight / gameScale * 2;

            auto tileX = std::floor(selectedTileX + selectedTileY + 1);
            auto tileY = std::floor(selectedTileY - selectedTileX + 1);

            if(!pathFindLookUp)
            {
                if(tileX >= 0 && tileX < this->getWidth() && tileY >= 0 && tileY < this->getHeight())
                {
                    tileMap[static_cast<int>(std::floor(selectedTileX + selectedTileY + 1))][static_cast<int>(std::floor(selectedTileY - selectedTileX + 1))]->tileId = tileToBePlaced;
                    roadTiling();
                    pathFindingInitialised = false;
                }
            }
            else
            {
                if(!clickedOnce)
                {
                    static constant::Vector2D end; // TODO: why this is here
                    static bool lockup = false;

                    if(tileX >= 0 && tileX < this->getWidth() && tileY >= 0 && tileY < this->getHeight())
                    {
                        if(!lockup)
                        {
                            startPath.x = tileX;
                            startPath.y = tileY;

                            lockup = true;

                            for(unsigned int i = 0; i < this->getWidth(); i++)
                            {
                                for(unsigned int j = 0; j < this->getHeight(); j++)
                                {
                                    if(*tileMap[i][j]->tileId == TileType::PATHFINDING)
                                        tileMap[i][j]->tileId = tilesLoader->getTile("Base Road RoundAbout");
                                }
                            }

                            roadTiling();
                        }
                        else
                        {
                            end.x = tileX;
                            end.y = tileY;

                            if(floatMapInitialised)
                            {
                                auto pathFinder = Path2D(MapFloat{floatMap, this->getWidth(), this->getHeight()});
                                auto path = pathFinder(startPath, end);

                                //std::cout << path.size() << std::endl;

                                //for(const auto& pos : path)
                                //{
                                //    std::cout << pos.x << ", " << pos.y << std::endl;
                                //}
                            }
                            /*
                            

                            auto path = pathFinder.getPath(startPath, end);

                            std::cout << path.length << std::endl;

                            //for(auto dir : path.dirList)
                            //{
                            //    switch (dir)
                            //    {
                            //        case PathFinder::Dir::NORTH:
                            //            std::cout << "NORTH ";
                            //            break;
                            //        case PathFinder::Dir::SOUTH:
                            //            std::cout << "SOUTH ";
                            //            break;
                            //        case PathFinder::Dir::EAST:
                            //            std::cout << "EAST ";
                            //            break;
                            //        case PathFinder::Dir::WEST:
                            //            std::cout << "WEST ";
                            //            break;
                            //    }
                            //
                            //    std::cout << std::endl;
                            //}

                            unsigned int x = startPath.x;
                            unsigned int y = startPath.y;

                            pathToRender = path;

                            for(auto dir : pathToRender.dirList)
                            {
                                
                                switch (dir)
                                {
                                case PathFinder::Dir::NORTH:
                                    x--;
                                    tileMap[x][y]->tileId = tilesLoader->getTile("Base Arrow Bot");
                                    break;
                                case PathFinder::Dir::SOUTH:
                                    x++;
                                    tileMap[x][y]->tileId = tilesLoader->getTile("Base Arrow Top");
                                    break;
                                case PathFinder::Dir::EAST:
                                    y++;
                                    tileMap[x][y]->tileId = tilesLoader->getTile("Base Arrow Left");
                                    break;
                                case PathFinder::Dir::WEST:
                                    y--;
                                    tileMap[x][y]->tileId = tilesLoader->getTile("Base Arrow Right");
                                    break;
                                case PathFinder::Dir::NONE:
                                    break;
                                }
                            }

                            meshUpdate = false;
                            */

                        lockup = false;
                        }
                    }
                }
            }

            clickedOnce = true;
        }

        if(!inputHandler->isButtonPressed(Qt::LeftButton))
            clickedOnce = false;
    }

    std::vector<constant::Vector2D> Map::createPathBetweenHouseAndShop()
    {
        static unsigned int rng = time(NULL);

        initPathFinding();

        auto pathFinder = Path2D(MapFloat{floatMap, this->getWidth(), this->getHeight()});

        std::vector<constant::Vector2D> housePos;
        std::vector<constant::Vector2D> shopPos;

        for(unsigned int i = 0; i < constraint.width; i++)
        {
            for(unsigned int j = 0; j < constraint.height; j++)
            {
                if(*tileMap[i][j]->tileId == TileType::HOUSE)
                    housePos.push_back({i, j});

                if(*tileMap[i][j]->tileId == TileType::SHOP)
                    shopPos.push_back({i, j});
            } 
        }

        if(housePos.size() > 0 && shopPos.size() > 0)
        {
            srand(rng);
            rng = rand();

            int s = rand() % housePos.size();
            int e = rand() % shopPos.size();
            
            //Create a path from a house position to a shop position
            auto path = pathFinder(housePos[s], shopPos[e]);

            //for(auto p : path)
            //    std::cout << p.x << " " << p.y << std::endl;

            return path;
        }

        return std::vector<constant::Vector2D>();
    }

    void Map::initPathFinding()
    {
        if(!pathFindingInitialised)
        {
            if(floatMapInitialised)
            {
                //TODO delete must be using the old height and width and not the current one
                //maybe make it impossible to change width and heigth and just reconstruct a new map
                for(unsigned int i = 0; i < this->getWidth(); i++)
                {
                    delete[] floatMap[i];
                }
                delete[] floatMap;
            }

            //todo delete float map when the map is destroyed

            floatMap = new const float*[this->getWidth()];
            for(unsigned int i = 0; i < this->getWidth(); i++)
            {
                const auto temp = new float[this->getHeight()];
                for(unsigned int j = 0; j < this->getHeight(); j++)
                {
                    temp[j] = *tileMap[i][j];
                }

                floatMap[i] = temp;
            }
            floatMapInitialised = true;

            // TODO
            //pathFinder.processMap(floatMap, this->getWidth(), this->getHeight());

            pathFindingInitialised = true;
        }
    }

    void Map::updateModelInfo()
    {
        modelInfo.nbVertices = 20 * constraint.width * constraint.height;
        modelInfo.nbIndices = 6 * constraint.width * constraint.height;

        if(modelInfo.vertices != nullptr)
            delete modelInfo.vertices;
        if(modelInfo.indices != nullptr)
            delete modelInfo.indices;

        modelInfo.vertices = new float [modelInfo.nbVertices];
        modelInfo.indices = new unsigned int [modelInfo.nbIndices];

        constant::ModelInfo tileModel;

        unsigned int i = 0;

        for(unsigned int x = 0; x < constraint.width; x++)
        {
            for(unsigned int y = 0; y < constraint.height; y++)
            {
                tileModel = tileMap[x][y]->tileId->getModelInfo();

                // Coord
                modelInfo.vertices[i * 20 + 0]  = x       ; modelInfo.vertices[i * 20 + 1]  = y       ; modelInfo.vertices[i * 20 + 2]  = 0.0f;
                modelInfo.vertices[i * 20 + 5]  = x + 1.0f; modelInfo.vertices[i * 20 + 6]  = y       ; modelInfo.vertices[i * 20 + 7]  = 0.0f;
                modelInfo.vertices[i * 20 + 10] = x       ; modelInfo.vertices[i * 20 + 11] = y + 1.0f; modelInfo.vertices[i * 20 + 12] = 0.0f;
                modelInfo.vertices[i * 20 + 15] = x + 1.0f; modelInfo.vertices[i * 20 + 16] = y + 1.0f; modelInfo.vertices[i * 20 + 17] = 0.0f;

                // Tex Coord
                modelInfo.vertices[i * 20 + 3]  = tileModel.vertices[3];  modelInfo.vertices[i * 20 + 4]  = tileModel.vertices[4];  
                modelInfo.vertices[i * 20 + 8]  = tileModel.vertices[8];  modelInfo.vertices[i * 20 + 9]  = tileModel.vertices[9];
                modelInfo.vertices[i * 20 + 13] = tileModel.vertices[13]; modelInfo.vertices[i * 20 + 14] = tileModel.vertices[14];
                modelInfo.vertices[i * 20 + 18] = tileModel.vertices[18]; modelInfo.vertices[i * 20 + 19] = tileModel.vertices[19];

                modelInfo.indices[i * 6 + 0] = 4 * i + 0; modelInfo.indices[i * 6 + 1] = 4 * i + 1; modelInfo.indices[i * 6 + 2] = 4 * i + 2;
                modelInfo.indices[i * 6 + 3] = 4 * i + 1; modelInfo.indices[i * 6 + 4] = 4 * i + 2; modelInfo.indices[i * 6 + 5] = 4 * i + 3;

                i++;
            }
        }
    }

    void Map::roadTiling()
    {
        // [Road Tiling]

        std::vector<Map::Tiles* > availableSpace;

        bool top;
        bool right;
        bool bottom;
        bool left;

        Map::Tiles *tile;

        const int mapWidth = this->getWidth(), mapHeight = this->getHeight();

        for(int i = 0; i < mapWidth; i++)
        {
            for(int j = 0; j < mapHeight; j++)
            {
                tile = tileMap[i][j];
                //if(*tile->tileId == TileType::HOUSE)
                //    tile->tileId = tilesLoader->getTile("Dirt");

                if(*tile->tileId == TileType::ROAD)
                {
                    top = false;
                    right = false;
                    bottom = false;
                    left = false;

                    if(i - 1 >= 0)
                    {
                        if(*tileMap[i - 1][j]->tileId == TileType::ROAD)
                            left = true;
                        else
                            availableSpace.push_back(tileMap[i - 1][j]);
                    }
                        
                    if(i + 1 < mapWidth)
                    {
                        if(*tileMap[i + 1][j]->tileId == TileType::ROAD)
                            right = true;
                        else
                            availableSpace.push_back(tileMap[i + 1][j]);
                    }
                        
                    if(j - 1 >= 0)
                    {
                        if(*tileMap[i][j - 1]->tileId == TileType::ROAD)
                            top = true;
                        else
                            availableSpace.push_back(tileMap[i][j - 1]);
                    }
                        
                    if(j + 1 < mapHeight)
                    {
                        if(*tileMap[i][j + 1]->tileId == TileType::ROAD)
                            bottom = true;
                        else
                            availableSpace.push_back(tileMap[i][j + 1]);
                    }
                        
                    if((top && bottom) && (!left && !right))
                    {
                        tile->tileId = tilesLoader->getTile("Base Road Right");
                    }
                    else if((!top && !bottom) && (left && right))
                    {
                        tile->tileId = tilesLoader->getTile("Base Road Top");
                    }
                    else if((top && !bottom) && (!left && !right))
                    {
                        tile->tileId = tilesLoader->getTile("Base Road Left End");
                    }
                    else if((!top && !bottom) && (left && !right))
                    {
                        tile->tileId = tilesLoader->getTile("Base Road Top End");
                    }
                    else if((!top && bottom) && (!left && !right))
                    {
                        tile->tileId = tilesLoader->getTile("Base Road Right End");
                    }
                    else if((!top && !bottom) && (!left && right))
                    {
                        tile->tileId = tilesLoader->getTile("Base Road Bot End");
                    }
                    else
                    {
                        tile->tileId = tilesLoader->getTile("Base Road RoundAbout");
                    }
                }
            }
        }

        meshUpdate = false;

        // [Road Tiling]
    }

    void Map::drawPath()
    {
        if(pathRoad)
        {
            /*
            unsigned int x = startPath.x;
            unsigned int y = startPath.y;

            for(auto dir : pathToRender.dirList)
            {
                
                switch (dir)
                {
                case PathFinder::Dir::NORTH:
                    x--;
                    tileMap[x][y]->tileId = tilesLoader->getTile("Base Arrow Bot");
                    break;
                case PathFinder::Dir::SOUTH:
                    x++;
                    tileMap[x][y]->tileId = tilesLoader->getTile("Base Arrow Top");
                    break;
                case PathFinder::Dir::EAST:
                    y++;
                    tileMap[x][y]->tileId = tilesLoader->getTile("Base Arrow Left");
                    break;
                case PathFinder::Dir::WEST:
                    y--;
                    tileMap[x][y]->tileId = tilesLoader->getTile("Base Arrow Right");
                    break;
                case PathFinder::Dir::NONE:
                    break;
                }
            }

            */

            meshUpdate = false;
        }
        else
        {
            for(unsigned int i = 0; i < this->getWidth(); i++)
            {
                for(unsigned int j = 0; j < this->getHeight(); j++)
                {
                    if(*tileMap[i][j]->tileId == TileType::PATHFINDING)
                        tileMap[i][j]->tileId = tilesLoader->getTile("Base Road RoundAbout");
                }
            }

            roadTiling();
        }
    }
}
