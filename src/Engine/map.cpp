#include "map.h"

#include "../UI/uiconstant.h"

namespace pg
{
    namespace
    {
        enum class ByteDirName : int 
        {
            TOP = 0b1000,
            RIGHT = 0b0100,
            BOTTOM = 0b0010,
            LEFT = 0b0001
        };

        struct ByteDir // byte representation 0btrbl
        {
            unsigned top    : 1;
            unsigned right  : 1;
            unsigned bottom : 1;
            unsigned left   : 1;

            ByteDir() : top(0), right(0), bottom(0), left(0) {}
            ByteDir(const int& rhs) { *this = rhs; }
            ByteDir(const ByteDir& rhs) { *this = rhs; }

            void operator=(const int& rhs)
            {
                top = (rhs & (int)ByteDirName::TOP) >> 3;
                right = (rhs & (int)ByteDirName::RIGHT) >> 2;
                bottom = (rhs & (int)ByteDirName::BOTTOM) >> 1;
                left = (rhs & (int)ByteDirName::LEFT);
            }

            void operator=(const ByteDir& rhs)
            {
                top = rhs.top; 
                right = rhs.right; 
                bottom = rhs.bottom; 
                left = rhs.left;
            }

            operator int() const
            {
                int data = 0;
                data |= top << 3; 
                data |= right << 2;
                data |= bottom << 1;
                data |= left;

                return data;
            }

            //const unsigned int count() const { return top + right + bottom + left; } 

            //unsigned int operator&(const int& rhs)
            //{
            //    unsigned int data = 0;
            //    data |= top << 3; 
            //    data |= right << 2;
            //    data |= bottom << 1;
            //    data |= left;
    //
            //    return data & rhs;
            //}
        };

        struct RoadConstruct
        {
            constant::Vector2D pos;
            ByteDir availableDir;  
        };
    }

    Map::Map(EntitySystem *ecs, TilesLoader *tilesLoader, const Map::MapConstraint& constraint) : ecs(ecs), tilesLoader(tilesLoader), constraint(constraint)
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

        generateRandomMap();
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
            
            auto screenWidthPtr = va_arg(args, UiSize*);
            auto screenHeightPtr = va_arg(args, UiSize*);
            auto gameScalePtr = va_arg(args, float*);
            auto camera = va_arg(args, Camera*);

            va_end(args);

            auto screenWidth = *screenWidthPtr;
            auto screenHeight = *screenHeightPtr;
            auto gameScale = *gameScalePtr;

            float selectedTileX = ((float)(mousePos.x() - screenWidth / 2.0f )) / (gameScale / 2.0f) + camera->Position.x() * screenWidth / gameScale;
            float selectedTileY = ((float)(screenHeight / 2.0f - mousePos.y())) / (gameScale / 4.0f) + camera->Position.y() * screenHeight / gameScale * 2;

            auto tileX = std::floor(selectedTileX + selectedTileY + 1);
            auto tileY = std::floor(selectedTileY - selectedTileX + 1);

            if(!pathFindLookUp)
            {
                if(tileX >= 0 && tileX < this->getWidth() && tileY >= 0 && tileY < this->getHeight())
                {
                    syncMutex.lock();
                    tileMap[static_cast<int>(std::floor(selectedTileX + selectedTileY + 1))][static_cast<int>(std::floor(selectedTileY - selectedTileX + 1))]->tileId = tileToBePlaced;
                    syncMutex.unlock();

                    roadTiling();
                    pathFindingInitialised = false;
                    initPathFinding();
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
                                //auto pathFinder = Path2D(MapFloat{floatMap, this->getWidth(), this->getHeight()});
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

        if(housePos.size() > 0 && shopPos.size() > 0)
        {
            srand(rng);
            rng = rand();

            int s = rand() % housePos.size();
            int e = rand() % shopPos.size();
            
            std::lock_guard<std::mutex> lock(syncMutex);
            //Create a path from a house position to a shop position
            auto path = pathFinder(housePos[s], shopPos[e]);

            return path;
        }

        return std::vector<constant::Vector2D>();
    }

    void Map::initPathFinding()
    {
        std::lock_guard<std::mutex> lock(syncMutex);

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

            // Set PathFinder for the current map
            pathFinder.setMap({floatMap, this->getWidth(), this->getHeight()});

            // Find all houses and shop of the Map and store them in a vector

            housePos.clear();
            housePos.shrink_to_fit();

            shopPos.clear();
            shopPos.shrink_to_fit();

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

    void Map::generateRandomMap()
    {
        std::queue<RoadConstruct> roadQueue;

        srand(constraint.seed);

        int xStart = rand() % constraint.width;
        int yStart = rand() % constraint.height;
        //int xStart = 0;
        //int yStart = 0;

        RoadConstruct startRoad = { constant::Vector2D(xStart, yStart), 0b0000 };

        startRoad.availableDir.left = (xStart - 1 >= 0);
        startRoad.availableDir.right = (xStart + 1 < static_cast<int>(constraint.width));
        startRoad.availableDir.top = (yStart - 1 >= 0); 
        startRoad.availableDir.bottom = (yStart + 1 < static_cast<int>(constraint.height));

        roadQueue.push(startRoad); // TODO use emplace and create a ctor and a copy ctor for RoadConstruct

        int openSpace = 1;

        const float fillRatio = 0.3; // TODO need to be added in constraint
        const int spaceToBeFilled = constraint.width * constraint.height * fillRatio; // TODO take in consideration unhabitable space such as water or mountain by scaning the map first for contiguous available space

        tileMap[xStart][yStart]->tileId = tilesLoader->getTile("Base Road RoundAbout");

        //While loop variables

        RoadConstruct road;    

        int nbAvailableDir = 0;
        ByteDirName possibleDir[4];

        ByteDirName currentDir;
        int length;

        ByteDir availableDir = 0b0000;

        int x;
        int y;

        while(openSpace < spaceToBeFilled) //TODO check if space to be filled can actually be filled or not
        {
            road = roadQueue.front();
            roadQueue.pop();

            //std::cout << "Road : " << road.pos.x << ", " << road.pos.y << std::endl;

            //auto nbDir = road.availableDir.count();

            // Find the direction to install road

            nbAvailableDir = 0;

            // Branchless version of possible dir finding
            availableDir = road.availableDir;
            for(int i = 0; i < 4; i++)
            {
                possibleDir[nbAvailableDir] = ByteDirName(((availableDir >> i) & 0b0001) * (1 << i));
                nbAvailableDir += (availableDir >> i) & 0b0001; 
            }

            //std::cout << nbAvailableDir << std::endl;
    //
            //for(int i = 0; i < nbAvailableDir; i++)
            //    std::cout << (int)possibleDir[i] << std::endl;

            // TODO till the edge case is not fix nbAvailableDir can be 0
            currentDir = possibleDir[rand() % nbAvailableDir];
            length = rand() % 4 + 1; // TODO insert max length in constraint

            // End road direction finding

            // Install road

            road.availableDir = road.availableDir & ~(int)currentDir;

            if(road.availableDir > 0)
                roadQueue.push(road);

            for(int i = 1; i <= length; i++)
            {
                x = road.pos.x;
                y = road.pos.y;

                switch (currentDir)
                {
                case ByteDirName::TOP:
                    y -= 1;
                    break;
                
                case ByteDirName::RIGHT:
                    x += 1;
                    break;

                case ByteDirName::BOTTOM:
                    y += 1;
                    break;

                case ByteDirName::LEFT:
                    x -= 1;
                    break;
                }

                // if out of bound -> break
                if(x < 0 || x >= static_cast<int>(constraint.width))
                    break;

                if(y < 0 || y >= static_cast<int>(constraint.height))
                    break;

                // if destination tile can t be modified -> break  [TODO] make a function to check if a tile is modifiable
                if(*tileMap[x][y]->tileId == TileType::ROAD)
                    break;

                // out of bound test
                availableDir.left = (x - 1 >= 0);
                availableDir.right = (x + 1 < static_cast<int>(constraint.width));
                availableDir.top = (y - 1 >= 0); 
                availableDir.bottom = (y + 1 < static_cast<int>(constraint.height));

                switch (currentDir)
                {
                case ByteDirName::TOP:
                    availableDir.bottom = 0;
                    if(availableDir.left) // if left column exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.left = *tileMap[x - 1][y]->tileId != TileType::ROAD;
                        if(!availableDir.left && *tileMap[x - 1][y + 1]->tileId == TileType::ROAD)
                            goto endRoad;
                    }

                    if(availableDir.right) // if right column exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.right = *tileMap[x + 1][y]->tileId != TileType::ROAD;
                        if(!availableDir.right && *tileMap[x + 1][y + 1]->tileId == TileType::ROAD)
                            goto endRoad;
                    }

                    // TODO Edge case afterwards
                    if(availableDir.top) // if top row exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.top = *tileMap[x][y - 1]->tileId != TileType::ROAD;
                    }                   
                    break;
                
                case ByteDirName::RIGHT:
                    availableDir.left = 0;
                    if(availableDir.top) // if top row exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.top = *tileMap[x][y - 1]->tileId != TileType::ROAD;
                        if(!availableDir.top && *tileMap[x - 1][y - 1]->tileId == TileType::ROAD)
                            goto endRoad;
                    }

                    if(availableDir.bottom) // if bottom row exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.bottom = *tileMap[x][y + 1]->tileId != TileType::ROAD;
                        if(!availableDir.bottom && *tileMap[x - 1][y + 1]->tileId == TileType::ROAD)
                            goto endRoad;
                    }

                    if(availableDir.right) // if right column exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.right = *tileMap[x + 1][y]->tileId != TileType::ROAD;
                    }                   
                    break;

                case ByteDirName::BOTTOM:
                    availableDir.top = 0;
                    if(availableDir.left) // if left column exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.left = *tileMap[x - 1][y]->tileId != TileType::ROAD;
                        if(!availableDir.left && *tileMap[x - 1][y - 1]->tileId == TileType::ROAD)
                            goto endRoad;
                    }

                    if(availableDir.right) // if right column exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.right = *tileMap[x + 1][y]->tileId != TileType::ROAD;
                        if(!availableDir.right && *tileMap[x + 1][y - 1]->tileId == TileType::ROAD)
                            goto endRoad;
                    }

                    if(availableDir.bottom) // if bottom row exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.bottom = *tileMap[x][y + 1]->tileId != TileType::ROAD;
                    }                   
                    break;

                case ByteDirName::LEFT:
                    availableDir.right = 0;
                    if(availableDir.top) // if top row exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.top = *tileMap[x][y - 1]->tileId != TileType::ROAD;
                        if(!availableDir.top && *tileMap[x + 1][y - 1]->tileId == TileType::ROAD)
                            goto endRoad;
                    }

                    if(availableDir.bottom) // if bottom row exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.bottom = *tileMap[x][y + 1]->tileId != TileType::ROAD;
                        if(!availableDir.bottom && *tileMap[x + 1][y + 1]->tileId == TileType::ROAD)
                            goto endRoad;
                    }

                    if(availableDir.left) // if left column exist
                    {
                        // if destination tile can t be modified -> dir is not available
                        availableDir.left = *tileMap[x - 1][y]->tileId != TileType::ROAD;
                    }                   
                    break;
                }
                
                if(availableDir > 0) // TODO remove this check once the edge case are resolved
                {
                    openSpace++;

                    tileMap[x][y]->tileId = tilesLoader->getTile("Base Road RoundAbout");

                    road = RoadConstruct( {constant::Vector2D(x, y), availableDir} );
                    roadQueue.push(road);
                }
            }

        endRoad:;
        }

        roadTiling();

        std::vector<Map::Tiles* > availableSpace;

        const int mapWidth = this->getWidth(), mapHeight = this->getHeight();

        for(int i = 0; i < mapWidth; i++)
        {
            for(int j = 0; j < mapHeight; j++)
            {
                const auto tile = tileMap[i][j];

                if(*tile->tileId == TileType::ROAD)
                {
                    if(i - 1 >= 0)
                        if(!(*tileMap[i - 1][j]->tileId == TileType::ROAD))
                            availableSpace.push_back(tileMap[i - 1][j]);
                        
                    if(i + 1 < mapWidth)
                        if(!(*tileMap[i + 1][j]->tileId == TileType::ROAD))
                            availableSpace.push_back(tileMap[i + 1][j]);
                        
                    if(j - 1 >= 0)
                        if(!(*tileMap[i][j - 1]->tileId == TileType::ROAD))
                            availableSpace.push_back(tileMap[i][j - 1]);
                        
                    if(j + 1 < mapHeight)
                        if(!(*tileMap[i][j + 1]->tileId == TileType::ROAD))
                            availableSpace.push_back(tileMap[i][j + 1]);
                }
            }
        }

        // [Road Tiling]
        // [Building Placement]
        int nbShop = availableSpace.size() / 10;
        int nbHouse = availableSpace.size() / 2;
        int blankSpace = availableSpace.size() - nbShop - nbHouse;
        int placement = 3;
        int placeResult;

        TilesLoader::TilesId* placeList[3] = {tilesLoader->getTile("Dirt"), tilesLoader->getTile("Base House"), tilesLoader->getTile("Base Shop")};
        TilesLoader::TilesId* placeItem;

        for(size_t i = 0; i < availableSpace.size(); i++)
        {
            if(placement > 0)
            {
                srand(rand());
                placeResult = rand() % placement;
                placeItem = placeList[placeResult];

                if(placeItem == tilesLoader->getTile("Dirt"))
                {
                    blankSpace--;
                    
                    if(blankSpace <= 0)
                    {
                        placement--;
                        if(placement > 0)
                            for(int j = 0; j <= placement; j++)
                                if(placeList[j] == placeItem)
                                    placeList[j] = placeList[placement];
                    }
                }
                else if(placeItem == tilesLoader->getTile("Base House"))
                {
                    availableSpace[i]->tileId = placeItem;

                    nbHouse--;
                    
                    if(nbHouse <= 0)
                    {
                        placement--;
                        if(placement > 0)
                            for(int j = 0; j <= placement; j++)
                                if(placeList[j] == placeItem)
                                    placeList[j] = placeList[placement];
                    }
                }
                else if(placeItem == tilesLoader->getTile("Base Shop"))
                {
                    availableSpace[i]->tileId = placeItem;

                    nbShop--;
                    
                    if(nbShop <= 0)
                    {
                        placement--;
                        if(placement > 0)
                            for(int j = 0; j <= placement; j++)
                                if(placeList[j] == placeItem)
                                    placeList[j] = placeList[placement];
                    }
                }
            }
        }
        // [Building Placement]

        initPathFinding();
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
