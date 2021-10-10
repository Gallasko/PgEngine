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

/*
PathFinder::~PathFinder()
{
    if(initialized)
    {
        delete[] currentPath;

        for(int i = width - 1; i >= 0; i--)
        {
            for(int j = height - 1; j >=0; j--)
            {
                delete nodeArray[i][j];
            }

            delete[] nodeArray[i];
            delete[] roadArray[i];
        }

        delete[] nodeArray;
        delete[] roadArray;
    }
}

void PathFinder::processMap(float **map, const int& width, const int& height)
{
    this->map = map;
    this->width = width;
    this->height = height;

    nodeList = findCoreNodes(map, width, height);
    nbNodes = nodeList.size();

    generateAllPath(&nodeList);

    initialized = true;
}

PathFinder::Path PathFinder::getPath(const constant::Vector2D& from, const constant::Vector2D& to)
{
    if(from.x < 0 || from.x >= width || from.y < 0 || from.y >= height)
        return PathFinder::Path();

    unsigned int startingNode = -1, endingNode = -1, startingNode2 = -1, endingNode2 = -1; 

    if(nodeArray[static_cast<int>(from.x)][static_cast<int>(from.y)] != nullptr)
    {
        startingNode = nodeArray[static_cast<int>(from.x)][static_cast<int>(from.y)]->id;
    }
    else if(roadArray[static_cast<int>(from.x)][static_cast<int>(from.y)] != nullptr)
    {
        startingNode = roadArray[static_cast<int>(from.x)][static_cast<int>(from.y)]->startNodeId;
        startingNode2 = roadArray[static_cast<int>(from.x)][static_cast<int>(from.y)]->endNodeId;
    }
    else
    {
        return PathFinder::Path();
    }

    if(nodeArray[static_cast<int>(to.x)][static_cast<int>(to.y)] != nullptr)
    {
        endingNode = nodeArray[static_cast<int>(to.x)][static_cast<int>(to.y)]->id;
    }
    else if(roadArray[static_cast<int>(to.x)][static_cast<int>(to.y)] != nullptr)
    {
        endingNode = roadArray[static_cast<int>(to.x)][static_cast<int>(to.y)]->startNodeId;
        endingNode2 = roadArray[static_cast<int>(to.x)][static_cast<int>(to.y)]->endNodeId;
    }
    else
    {
        return PathFinder::Path();
    }

    //startingNode * nbNodes + endingNode
    unsigned int len1 = -1, len2 = -1, len3 = -1, len4 = -1;
    len1 = currentPath[startingNode * nbNodes + endingNode].length;

    if(endingNode2 != -1)
        len2 = currentPath[startingNode * nbNodes + endingNode2].length;

    if(startingNode2 != -1)
        len3 = currentPath[startingNode2 * nbNodes + endingNode].length;

    if(startingNode2 != -1 && endingNode2 != -1)
        len4 = currentPath[startingNode2 * nbNodes + endingNode2].length;

    if(len1 == static_cast<unsigned int>(-1) && len2 == static_cast<unsigned int>(-1) && len3 == static_cast<unsigned int>(-1) && len4 == static_cast<unsigned int>(-1))
        return PathFinder::Path();

    if(len1 <= len2 && len1 <= len3 && len1 <= len4)
        return makePath(startingNode, endingNode);
    else if(len2 <= len1 && len2 <= len3 && len2 <= len4)
        return makePath(startingNode, endingNode);
    else if(len3 <= len1 && len3 <= len2 && len3 <= len4)
        return makePath(startingNode, endingNode);
    else if(len4 <= len1 && len4 <= len2 && len4 <= len3)
        return makePath(startingNode, endingNode);
    else
        return PathFinder::Path();
}

std::vector<PathFinder::Node* > PathFinder::findCoreNodes(float **array, const int& width, const int& height)
{
    float top = 0, right = 0, bottom = 0, left = 0;

    std::vector<PathFinder::Node* > nodes;

    unsigned int nodeId = 0;
    nodeArray = new PathFinder::Node**[width];
    roadArray = new PathFinder::Road**[width];
    
    float** resultArray = new float*[width];

    for(int i = 0; i < width; i++)
    {
        resultArray[i] = new float[height];
        nodeArray[i] = new PathFinder::Node*[height];
        roadArray[i] = new PathFinder::Road*[height];

        for(int j = 0; j < height; j++)
        {
            resultArray[i][j] = 0.0f;
            nodeArray[i][j] = nullptr;
            roadArray[i][j] = nullptr;

            if(i - 1 >= 0)
                top = array[i - 1][j];
            else
                top = 0;
            
            if(i + 1 < width)
                bottom = array[i + 1][j];
            else
                bottom = 0;

            if(j - 1 >= 0)
                left = array[i][j - 1];
            else
                left = 0;

            if(j + 1 < height)
                right = array[i][j + 1];
            else
                right = 0;

            if(!((top && bottom && !left && !right) || (!top && !bottom && left && right) || (!top && !bottom && !left && !right)) && array[i][j])
            {
                resultArray[i][j] = 1 + nodeId;
                PathFinder::Node *node = new PathFinder::Node(nodeId, i, j, top + left + right + bottom);

                int k = 0;

                if(top)
                {
                    node->path[k].startNodeId = nodeId;
                    node->path[k++].dir = PathFinder::Dir::NORTH;
                }
                    
                if(left)
                {
                    node->path[k].startNodeId = nodeId;
                    node->path[k++].dir = PathFinder::Dir::WEST;
                }

                if(bottom)
                {
                    node->path[k].startNodeId = nodeId;
                    node->path[k++].dir = PathFinder::Dir::SOUTH;
                }
                    
                if(right)
                {
                    node->path[k].startNodeId = nodeId;
                    node->path[k++].dir = PathFinder::Dir::EAST;
                }

                nodes.push_back(node);
                nodeArray[i][j] = node;
                
                nodeId++;
            }
        }
    }

    for(auto node : nodes)
    {
        for(int a = 0; a < node->nbAdjNodes; a++)
        {
            int i = node->x;
            int j = node->y;
            int n = 0;
            PathFinder::Dir dir = node->path[a].dir;

            do
            {
                n++;

                switch (dir)
                {
                case PathFinder::Dir::NORTH:
                    i--;
                    break;
                case PathFinder::Dir::SOUTH:
                    i++;
                    break;
                case PathFinder::Dir::EAST:
                    j++;
                    break;
                case PathFinder::Dir::WEST:
                    j--;
                    break;
                case PathFinder::Dir::NONE:
                    break;
                }

                if(resultArray[i][j] == 0)
                    roadArray[i][j] = &node->path[a];

            } while (resultArray[i][j] == 0);

            node->path[a].length = n;
            if(nodeArray[i][j] != nullptr)
                node->path[a].endNodeId = nodeArray[i][j]->id;
        }
    }

    for(int i = width - 1; i >= 0; i--)
        delete[] resultArray[i];

    delete[] resultArray;

    return nodes;
}

void PathFinder::generateAllPath(std::vector<PathFinder::Node* > *nodes)
{
    std::cout << "Allocation Begin" << std::endl;

    PathFinder::Path *pathToBeEvaluated = new PathFinder::Path[nbNodes * nbNodes * nbNodes];
    //PathFinder::Road pathToBeEvaluated[nbNodes * nbNodes * nbNodes];

    std::cout << "Allocation done" << std::endl;

    unsigned int n = 0;

    currentPath = new PathFinder::Path[nbNodes * nbNodes];
    PathFinder::Path *newPath = new PathFinder::Path[nbNodes * nbNodes];

    for(unsigned short i = 0; i < nbNodes; i++)
    {
        for(unsigned short j = 0; j < nbNodes; j++)
        {
            currentPath[i * nbNodes + j].length = static_cast<unsigned int>(-1);
            newPath[i * nbNodes + j].length = static_cast<unsigned int>(-1);

            currentPath[i * nbNodes + j].startNodeId = i;
            newPath[i * nbNodes + j].startNodeId = i;

            currentPath[i * nbNodes + j].endNodeId = j;
            newPath[i * nbNodes + j].endNodeId = j;

            //if(i == j)
            //{
            //    pathToBeEvaluated[n].startNodeId = i;
            //    pathToBeEvaluated[n].endNodeId = j;
            //    pathToBeEvaluated[n].length = 0;
//
            //    n++;
            //}
               
        }
        
    }

    for(auto node : *nodes)
        for(unsigned short i = 0; i < node->nbAdjNodes; i++)
            pathToBeEvaluated[n++] = node->path[i];

    PathFinder::Path tempPath;
    unsigned int x, y;

    do
    {
        std::cout << n << std::endl;

        for(unsigned int i = 0; i < n; i++)
        {
            tempPath = pathToBeEvaluated[i];
            x = tempPath.startNodeId;
            y = tempPath.endNodeId; 

            if(tempPath.length < currentPath[x * nbNodes + y].length)
                newPath[x * nbNodes + y] = tempPath;
        }

        n = 0;

        for(unsigned short i = 0; i < nbNodes; i++)
        {
            for(unsigned short j = 0; j < nbNodes; j++)
            {   
                if(newPath[i * nbNodes + j].length != static_cast<unsigned int>(-1))
                {
                    for(unsigned short k = 0; k < nbNodes; k++)
                    {
                        if(currentPath[i * nbNodes + k].length != static_cast<unsigned int>(-1))
                        {
                            pathToBeEvaluated[n] = currentPath[i * nbNodes + k] - newPath[i * nbNodes + j];
                            pathToBeEvaluated[n].startNodeId = currentPath[i * nbNodes + k].endNodeId;
                            pathToBeEvaluated[n].endNodeId = newPath[i * nbNodes + j].endNodeId;
                            n++;
                        
                            pathToBeEvaluated[n] = newPath[i * nbNodes + j] - currentPath[i * nbNodes + k];
                            pathToBeEvaluated[n].endNodeId = currentPath[i * nbNodes + k].endNodeId;
                            pathToBeEvaluated[n].startNodeId = newPath[i * nbNodes + j].endNodeId;
                            n++;
                        }
                    }

                    for(unsigned short k = j + 1; k < nbNodes; k++)
                    {
                        if(newPath[i * nbNodes + k].length != static_cast<unsigned int>(-1))
                        {
                            pathToBeEvaluated[n] = newPath[i * nbNodes + k] - newPath[i * nbNodes + j];
                            pathToBeEvaluated[n].startNodeId = newPath[i * nbNodes + k].endNodeId;
                            pathToBeEvaluated[n].endNodeId = newPath[i * nbNodes + j].endNodeId;
                            n++;

                            pathToBeEvaluated[n] = newPath[i * nbNodes + j] - newPath[i * nbNodes + k];
                            pathToBeEvaluated[n].endNodeId = newPath[i * nbNodes + k].endNodeId;
                            pathToBeEvaluated[n].startNodeId = newPath[i * nbNodes + j].endNodeId;
                            n++;
                        }
                    }
                }
            }

            for(unsigned short j = 0; j < nbNodes; j++)
            {
                if(newPath[i * nbNodes + j].length != static_cast<unsigned int>(-1))
                {
                    currentPath[i * nbNodes + j] = newPath[i * nbNodes + j];
                    newPath[i * nbNodes + j].length = static_cast<unsigned int>(-1);
                    newPath[i * nbNodes + j].dirList.clear();
                }
            }     
        }

    } while(n > 0);

    //delete[] pathToBeEvaluated;
    //delete[] newPath;

}

PathFinder::Path PathFinder::makePath(const unsigned int& startingNode, const unsigned int& endNode)
{
    Node *sNode = nodeList[startingNode];
    Node *eNode = nodeList[endNode];

    int currentX = sNode->x;
    int currentY = sNode->y;

    PathFinder::Path path;

    PathFinder::Path currentRoad;
    PathFinder::Node *currentNode;

    currentRoad = currentPath[startingNode * nbNodes + endNode];

    return currentRoad;

    
    //do
    //{
    //    currentNode = nodeArray[currentX][currentY];
    //    
//
    //    switch (currentRoad.dir)
    //    {
    //    case PathFinder::Dir::NORTH:
    //        currentX -= currentRoad.length;
    //        break;
    //    case PathFinder::Dir::SOUTH:
    //        currentX += currentRoad.length;
    //        break;
    //    case PathFinder::Dir::EAST:
    //        currentY += currentRoad.length;
    //        break;
    //    case PathFinder::Dir::WEST:
    //        currentY -= currentRoad.length;
    //        break;
    //    case PathFinder::Dir::NONE:
    //        return PathFinder::Path();
    //        break;
    //    }
//
    //    path.length += currentRoad.length;
    //    path.dirList.insert(path.dirList.end(), currentRoad.length, currentRoad.dir);
//
    //} while (currentX != eNode->x && currentY != eNode->y);
    
    return path;
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

void Map::runPathFinding(Input* inputHandler, double deltaTime...)
{
    if(inputHandler->isButtonPressed(Qt::LeftButton))
    {
        if(!pathFindingInitialised)
        {
            if(floatMapInitialised)
            {
                //TODO delete must be using the old height and width and note the current one
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

                        for(int i = 0; i < this->getWidth(); i++)
                        {
                            for(int j = 0; j < this->getHeight(); j++)
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

                            std::cout << path.size() << std::endl;

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

void Map::roadTiling()
{
    // [Road Tiling]

    std::vector<Map::Tiles* > availableSpace;

    bool top;
    bool right;
    bool bottom;
    bool left;

    Map::Tiles *tile;

    for(int i = 0; i < this->getWidth(); i++)
    {
        for(int j = 0; j < this->getHeight(); j++)
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
                    
                if(i + 1 < this->getWidth())
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
                    
                if(j + 1 < this->getHeight())
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
        unsigned int x = startPath.x;
        unsigned int y = startPath.y;

        /*

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
        for(int i = 0; i < this->getWidth(); i++)
        {
            for(int j = 0; j < this->getHeight(); j++)
            {
                if(*tileMap[i][j]->tileId == TileType::PATHFINDING)
                    tileMap[i][j]->tileId = tilesLoader->getTile("Base Road RoundAbout");
            }
        }

        roadTiling();
    }
}