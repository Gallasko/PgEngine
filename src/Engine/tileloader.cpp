#include "tileloader.h"

#define NBROWTEXTUREATLAS 16.0f
#define SPRITESIZE 64.0f
#define ATLASSIZE 1024.0f

TilesLoader::TilesId::TilesId()
{
	initializeOpenGLFunctions(); 

	VAO = new QOpenGLVertexArrayObject();
	VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

	VAO->create();
	VBO->create();
	EBO->create();
}

TilesLoader::TilesId::~TilesId()
{
	delete VAO;
	delete VBO;
	delete EBO;
}

void TilesLoader::TilesId::setMesh(int textureId)
{
	int column = textureId % (int)NBROWTEXTUREATLAS;
    float xMin = (column * SPRITESIZE) / ATLASSIZE;
    float xMax = ((column + 1) * SPRITESIZE) / ATLASSIZE;

    int row = (float)textureId / NBROWTEXTUREATLAS;
    float yMin = (row * SPRITESIZE)  / ATLASSIZE;
    float yMax = ((row + 1) * SPRITESIZE)  / ATLASSIZE;

	//texpos x                 texpos y
	modelInfo.vertices[3] =  xMin; modelInfo.vertices[4] =  yMin;   
	modelInfo.vertices[8] =  xMax; modelInfo.vertices[9] =  yMin;
	modelInfo.vertices[13] = xMin; modelInfo.vertices[14] = yMax;
	modelInfo.vertices[18] = xMax; modelInfo.vertices[19] = yMax;

    VAO->bind();

    // position attribute
    glEnableVertexAttribArray(0);
    VBO->bind();
    VBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
    VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // texture coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    EBO->bind();
    EBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
    EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

    VAO->release();
}

void TilesLoader::TilesId::setType(std::string type)
{
	if(type == "Blank")
		tileType = TileType::BLANK;
	else if(type == "House")
		tileType = TileType::HOUSE;
	else if(type == "Shop")
		tileType = TileType::SHOP;
	else if(type == "Road")
		tileType = TileType::ROAD;
	else if(type == "Misc")
		tileType = TileType::MISC;
	else if(type == "Pathfinding")
		tileType = TileType::PATHFINDING;
	else
		tileType = TileType::BLANK;
}

TilesLoader::TilesLoader(std::string tilesFolder) : nbTilesId(0)
{
	std::ifstream f;
	TilesLoader::TilesId *newTile;

	f.open(tilesFolder + std::to_string(nbTilesId) + ".tile", std::ifstream::in);

	while (f.is_open())
	{
		newTile = new TilesLoader::TilesId();

		newTile->setId(nbTilesId);

		for(std::string line; std::getline(f, line); )
		{
			if(line == "Tile Name")
				if (std::getline(f, line))
					newTile->setName(line);

			if(line == "Texture")
				if (std::getline(f, line))
					newTile->setMesh(std::stoi(line));

			if(line == "Type")
				if (std::getline(f, line))
					newTile->setType(line);
		}

		tilesList.push_back(newTile);
		tilesDict[newTile->getName()] = nbTilesId;

		f.close();
		nbTilesId++; 
		f.open(tilesFolder + std::to_string(nbTilesId) + ".tile", std::ifstream::in);
	}
}

TilesLoader::~TilesLoader()
{
	tilesList.clear();
    tilesList.shrink_to_fit();
	//std::vector<TilesLoader::TilesId* >().swap(tilesList); //delete block list
}

TilesLoader::TilesId* TilesLoader::getTile(int id) const
{
	if(id < nbTilesId)
		return tilesList[id];
	else
		return NULL;
}

TilesLoader::TilesId* TilesLoader::getTile(std::string tileName) const
{
	auto it = tilesDict.find(tileName);

	if(it != tilesDict.end())
		return tilesList[tilesDict.at(tileName)];
	else
		return NULL;
}