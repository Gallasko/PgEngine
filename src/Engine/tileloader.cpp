#include "tileloader.h"

#define NBROWTEXTUREATLAS 16.0f
#define SPRITESIZE 64.0f
#define ATLASSIZE 1024.0f

TilesLoader::Tiles::Tiles()
{
	initializeOpenGLFunctions(); 

	VAO = new QOpenGLVertexArrayObject();
	VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

	VAO->create();
	VBO->create();
	EBO->create();
}

TilesLoader::Tiles::~Tiles()
{
	delete VAO;
	delete VBO;
	delete EBO;
}

void TilesLoader::Tiles::setMesh(int textureId)
{
	int column = textureId % (int)NBROWTEXTUREATLAS;
    float xMin = (column * SPRITESIZE) / ATLASSIZE;
    float xMax = ((column + 1) * SPRITESIZE) / ATLASSIZE;

    int row = (float)textureId / NBROWTEXTUREATLAS;
    float yMin = (row * SPRITESIZE)  / ATLASSIZE;
    float yMax = ((row + 1) * SPRITESIZE)  / ATLASSIZE;

	auto tileVertices = new float[20];

    //                 x                         y                         z                      texpos x                 texpos y
	tileVertices[0] =  -0.5f; tileVertices[1] =   0.5f; tileVertices[2] =  0.0f; tileVertices[3] =  xMin; tileVertices[4] =  yMin;   
	tileVertices[5] =   0.5f; tileVertices[6] =   0.5f; tileVertices[7] =  0.0f; tileVertices[8] =  xMax; tileVertices[9] =  yMin;
	tileVertices[10] = -0.5f; tileVertices[11] = -0.5f; tileVertices[12] = 0.0f; tileVertices[13] = xMin; tileVertices[14] = yMax;
	tileVertices[15] =  0.5f; tileVertices[16] = -0.5f; tileVertices[17] = 0.0f; tileVertices[18] = xMax; tileVertices[19] = yMax;

	unsigned int nbTileVertices = 20;

	auto tileVerticesIndice = new unsigned int[6];

	tileVerticesIndice[0] = 0; tileVerticesIndice[1] = 1; tileVerticesIndice[2] = 2;
	tileVerticesIndice[3] = 1; tileVerticesIndice[4] = 2; tileVerticesIndice[5] = 3;

	unsigned int nbOfElements = 6;

    VAO->bind();

    // position attribute
    glEnableVertexAttribArray(0);
    VBO->bind();
    VBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
    VBO->allocate(tileVertices, nbTileVertices * sizeof(float));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // texture coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    EBO->bind();
    EBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
    EBO->allocate(tileVerticesIndice, nbOfElements * sizeof(unsigned int));

    VAO->release();
}

void TilesLoader::Tiles::setType(std::string type)
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
	else
		tileType = TileType::BLANK;
}

TilesLoader::TilesLoader(std::string tilesFolder) : nbTilesId(0)
{
	std::ifstream f;
	TilesLoader::Tiles *newTile;

	f.open(tilesFolder + std::to_string(nbTilesId) + ".tile", std::ifstream::in);

	while (f.is_open())
	{
		std::cout << "Open file: " << tilesFolder + std::to_string(nbTilesId) + ".tile" << std::endl;
		newTile = new TilesLoader::Tiles();

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

		std::cout << newTile->getName() << std::endl;

		f.close();
		nbTilesId++; 
		f.open(tilesFolder + std::to_string(nbTilesId) + ".tile", std::ifstream::in);
	}
}

TilesLoader::~TilesLoader()
{
	std::vector<TilesLoader::Tiles* >().swap(tilesList); //delete block list
}

TilesLoader::Tiles* TilesLoader::getTile(int id) const
{
	if(id < nbTilesId)
		return tilesList[id];
	else
		return NULL;
}

TilesLoader::Tiles* TilesLoader::getTile(std::string tileName) const
{
	auto it = tilesDict.find(tileName);

	if(it != tilesDict.end())
		return tilesList[tilesDict.at(tileName)];
	else
		return NULL;
}