#include "map.h"

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
