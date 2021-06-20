#include "map.h"

/*
Map::Map(EntitySystem *ecs, TilesLoader *tilesLoader, unsigned int width, unsigned int height, unsigned int nbMaxRoad) : ecs(ecs), tilesLoader(tilesLoader), width(width), height(height), nbMaxRoad(nbMaxRoad)
{
    tileMap = new EntitySystem::Entity**[width];
    for(int i = 0; i < width; i++)
    {
        tileMap[i] = new EntitySystem::Entity*[height];

        for(int j = 0; j < height; j++)
        {
            tileMap[i][j] = ecs->createEntity();
            ecs->attach<Position>(tileMap[i][j], {static_cast<float>(i), static_cast<float>(j)});
            ecs->attach<TileHolder>(tileMap[i][j], { tilesLoader->getTile("Dirt") } );
        }
    }

    // [Road Creation]

    srand(time(NULL));

    unsigned int maxIteration = nbMaxRoad * 40;
    unsigned int currentIteration = 0;
    unsigned int nbRoad = 1;
    unsigned int nbSelectedTiles;
    unsigned int dir;
    int len;
    int x, y;
    bool housePercuted = false;

    x = rand() % width;
    srand(rand());
    y = rand() % height;

    auto tile = tileMap[x][y];
    tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Top");

    std::vector<EntitySystem::Entity* > roadVec;
    roadVec.push_back(tile);

    do
    {
        srand(rand());
        dir = rand() % 4;
        srand(rand());
        len = rand() % 5 + 3;
        srand(rand());
        nbSelectedTiles = rand() % nbRoad;

        tile = roadVec[nbSelectedTiles];

        x = tile->get<Position>()->x;
        y = tile->get<Position>()->y;

        housePercuted = false;

        for(int i = 1; i < len; i++)
        {
            if(!housePercuted)
            {
                switch(dir)
                {
                case 0: //Left
                    if(x - i >= 0)
                    {
                        tile = tileMap[x - i][y];
                        if(*tile->get<TileHolder>()->tileId == TileType::HOUSE)
                        {
                            housePercuted = true;
                            break;
                        }

                        if(*tile->get<TileHolder>()->tileId == TileType::ROAD)
                            break;

                        tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Top");
                        roadVec.push_back(tile);
                        nbRoad++;

                        if(i == 0 || i == len - 1)
                            break;

                        if(y - 1 >= 0)
                        {
                            tile = tileMap[x - i][y - 1];
                            if(!(*tile->get<TileHolder>()->tileId == TileType::ROAD || *tile->get<TileHolder>()->tileId == TileType::HOUSE))
                                tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base House");
                        }

                        if(y + 1 < width)
                        {
                            tile = tileMap[x - i][y + 1];
                            if(!(*tile->get<TileHolder>()->tileId == TileType::ROAD || *tile->get<TileHolder>()->tileId == TileType::HOUSE))
                                tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base House");
                        }
                    }
                break;

                case 1: //Up
                    if(y - i >= 0)
                    {
                        tile = tileMap[x][y - i];
                        if(*tile->get<TileHolder>()->tileId == TileType::HOUSE)
                        {
                            housePercuted = true;
                            break;
                        }

                        if(*tile->get<TileHolder>()->tileId == TileType::ROAD)
                            break;
                        
                        tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Top");
                        roadVec.push_back(tile);
                        nbRoad++;

                        if(i == 0 || i == len - 1)
                            break;

                        if(x - 1 >= 0)
                        {
                            tile = tileMap[x - 1][y - i];
                            if(!(*tile->get<TileHolder>()->tileId == TileType::ROAD || *tile->get<TileHolder>()->tileId == TileType::HOUSE))
                                tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base House");
                        }

                        if(x + 1 < height)
                        {
                            tile = tileMap[x + 1][y - i];
                            if(!(*tile->get<TileHolder>()->tileId == TileType::ROAD || *tile->get<TileHolder>()->tileId == TileType::HOUSE))
                                tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base House");
                        }
                    }
                break;

                case 2: //Right
                    if(x + i < width)
                    {
                        tile = tileMap[x + i][y];
                        if(*tile->get<TileHolder>()->tileId == TileType::HOUSE)
                        {
                            housePercuted = true;
                            break;
                        }

                        if(*tile->get<TileHolder>()->tileId == TileType::ROAD)
                            break;
                        
                        tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Top");
                        roadVec.push_back(tile);
                        nbRoad++;

                        if(i == 0 || i == len - 1)
                            break;

                        if(y - 1 >= 0)
                        {
                            tile = tileMap[x + i][y - 1];
                            if(!(*tile->get<TileHolder>()->tileId == TileType::ROAD || *tile->get<TileHolder>()->tileId == TileType::HOUSE))
                                tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base House");
                        }

                        if(y + 1 < height)
                        {
                            tile = tileMap[x + i][y + 1];
                            if(!(*tile->get<TileHolder>()->tileId == TileType::ROAD || *tile->get<TileHolder>()->tileId == TileType::HOUSE))
                                tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base House");
                        }
                    }
                break;

                case 3: //Down
                    if(y + i < height)
                    {
                        tile = tileMap[x][y + i];
                        if(*tile->get<TileHolder>()->tileId == TileType::HOUSE)
                        {
                            housePercuted = true;
                            break;
                        }

                        if(*tile->get<TileHolder>()->tileId == TileType::ROAD)
                            break;
                        
                        tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Top");
                        roadVec.push_back(tile);
                        nbRoad++;

                        if(i == 0 || i == len - 1)
                            break;

                        if(x - 1 >= 0)
                        {
                            tile = tileMap[x - 1][y + i];
                            if(!(*tile->get<TileHolder>()->tileId == TileType::ROAD || *tile->get<TileHolder>()->tileId == TileType::HOUSE))
                                tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base House");
                        }

                        if(x + 1 < width)
                        {
                            tile = tileMap[x + 1][y + i];
                            if(!(*tile->get<TileHolder>()->tileId == TileType::ROAD || *tile->get<TileHolder>()->tileId == TileType::HOUSE))
                                tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base House");
                        }
                    }
                break;
                }
            }
        }

        currentIteration++;
    } while (nbRoad < nbMaxRoad && currentIteration < maxIteration);

    // [Road Creation]

    // [Road Tiling]

    std::vector<EntitySystem::Entity* > availableSpace;

    bool top;
    bool right;
    bool bottom;
    bool left;

    for(int i = 0; i < width; i++)
    {
        for(int j = 0; j < height; j++)
        {
            tile = tileMap[i][j];
            if(*tile->get<TileHolder>()->tileId == TileType::HOUSE)
                tile->get<TileHolder>()->tileId = tilesLoader->getTile("Dirt");

            if(*tile->get<TileHolder>()->tileId == TileType::ROAD)
            {
                top = false;
                right = false;
                bottom = false;
                left = false;

                if(i - 1 >= 0)
                {
                    if(*tileMap[i - 1][j]->get<TileHolder>()->tileId == TileType::ROAD)
                        left = true;
                    else
                        availableSpace.push_back(tileMap[i - 1][j]);
                }
                    
                if(i + 1 < width)
                {
                    if(*tileMap[i + 1][j]->get<TileHolder>()->tileId == TileType::ROAD)
                        right = true;
                    else
                        availableSpace.push_back(tileMap[i + 1][j]);
                }
                    
                if(j - 1 >= 0)
                {
                    if(*tileMap[i][j - 1]->get<TileHolder>()->tileId == TileType::ROAD)
                        top = true;
                    else
                        availableSpace.push_back(tileMap[i][j - 1]);
                }
                    
                if(j + 1 < height)
                {
                    if(*tileMap[i][j + 1]->get<TileHolder>()->tileId == TileType::ROAD)
                        bottom = true;
                    else
                        availableSpace.push_back(tileMap[i][j + 1]);
                }
                    
                if((top && bottom) && (!left && !right))
                {
                    tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Right");
                }
                else if((!top && !bottom) && (left && right))
                {
                    tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Top");
                }
                else if((top && !bottom) && (!left && !right))
                {
                    tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Left End");
                }
                else if((!top && !bottom) && (left && !right))
                {
                    tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Top End");
                }
                else if((!top && bottom) && (!left && !right))
                {
                    tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Right End");
                }
                else if((!top && !bottom) && (!left && right))
                {
                    tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road Bot End");
                }
                else
                {
                    tile->get<TileHolder>()->tileId = tilesLoader->getTile("Base Road RoundAbout");
                }
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

    TilesLoader::Tiles* placeList[3] = {tilesLoader->getTile("Dirt"), tilesLoader->getTile("Base House"), tilesLoader->getTile("Base Shop")};
    TilesLoader::Tiles* placeItem;

    for(int i = 0; i < availableSpace.size(); i++)
    {
        if(placement > 0)
        {
            srand(rand());
            placeResult = rand() % placement;

            placeItem = placeList[placeResult];
            availableSpace[i]->get<TileHolder>()->tileId = placeItem;

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

}
*/

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

    for(int i = 0; i < constraint.width; i++)
    {
        tileMap[i] = new Map::Tiles*[constraint.height];

        for(int j = 0; j < constraint.height; j++)
            tileMap[i][j] = new Tiles(i, j, tilesLoader->getTile("Dirt"));
    }

    noiseGenerator = new NoiseGenerator(constraint.seed);
    noiseGenerator->setParameters(constraint.noiseParam);

    srand(constraint.seed);

    for(int i = 0; i < constraint.width; i++)
    {
        for(int j = 0; j < constraint.height; j++)
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
}

Map::~Map()
{
    for(int i = constraint.width - 1; i >= 0; i--)
        delete[] tileMap[i];

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

    for(int x = 0; x < constraint.width; x++)
    {
        for(int y = 0; y < constraint.height; y++)
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