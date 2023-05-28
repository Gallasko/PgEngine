// Todo

// #include "tileloader.h"

// #include "../Files/parser.h"
// #include "../logger.h"

// namespace pg
// {
// 	namespace
// 	{
// 		//TODO make this a param of the atlas somewhere in a file
// 		#define NBROWTEXTUREATLAS 16.0f
// 		#define SPRITESIZE 64.0f
// 		#define ATLASSIZE 1024.0f

// 		static constexpr char const * DOM = "Tile Loader";
// 	}

// 	TilesLoader::TilesId::TilesId()
// 	{
// 		LOG_THIS_MEMBER(DOM);

// 		initializeOpenGLFunctions(); 

// 		VAO = new QOpenGLVertexArrayObject();
// 		VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
// 		EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

// 		VAO->create();
// 		VBO->create();
// 		EBO->create();
// 	}

// 	TilesLoader::TilesId::~TilesId()
// 	{
// 		LOG_THIS_MEMBER(DOM);

// 		delete VAO;
// 		delete VBO;
// 		delete EBO;
// 	}

// 	void TilesLoader::TilesId::setMesh(int textureId)
// 	{
// 		LOG_THIS_MEMBER(DOM);

// 		int column = textureId % (int)NBROWTEXTUREATLAS;
// 		float xMin = (column * SPRITESIZE) / ATLASSIZE;
// 		float xMax = ((column + 1) * SPRITESIZE) / ATLASSIZE;

// 		int row = (float)textureId / NBROWTEXTUREATLAS;
// 		float yMin = (row * SPRITESIZE)  / ATLASSIZE;
// 		float yMax = ((row + 1) * SPRITESIZE)  / ATLASSIZE;

// 		//texpos x                 texpos y
// 		modelInfo.vertices[3] =  xMin; modelInfo.vertices[4] =  yMin;   
// 		modelInfo.vertices[8] =  xMax; modelInfo.vertices[9] =  yMin;
// 		modelInfo.vertices[13] = xMin; modelInfo.vertices[14] = yMax;
// 		modelInfo.vertices[18] = xMax; modelInfo.vertices[19] = yMax;

// 		VAO->bind();

// 		// position attribute
// 		glEnableVertexAttribArray(0);
// 		VBO->bind();
// 		VBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
// 		VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

// 		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

// 		// texture coord attribute
// 		glEnableVertexAttribArray(1);
// 		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

// 		EBO->bind();
// 		EBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
// 		EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

// 		VAO->release();
// 	}

// 	void TilesLoader::TilesId::setType(std::string type)
// 	{
// 		LOG_THIS_MEMBER(DOM);

// 		if(type == "Blank")
// 			tileType = TileType::BLANK;
// 		else if(type == "House")
// 			tileType = TileType::HOUSE;
// 		else if(type == "Shop")
// 			tileType = TileType::SHOP;
// 		else if(type == "Road")
// 			tileType = TileType::ROAD;
// 		else if(type == "Misc")
// 			tileType = TileType::MISC;
// 		else if(type == "Pathfinding")
// 			tileType = TileType::PATHFINDING;
// 		else
// 			tileType = TileType::BLANK;
// 	}

// 	TilesLoader::TilesLoader(const std::string& tilesFolder) : nbTilesId(0)
// 	{
// 		LOG_THIS_MEMBER(DOM);

// 		std::shared_ptr<TilesLoader::TilesId> newTile;

// 		std::vector<TextFile> folder = ResourceAccessor::openTextFolder(tilesFolder);

// 		for(auto file : folder)
// 		{
// 			newTile = std::make_shared<TilesLoader::TilesId>();
// 			newTile->setId(nbTilesId);

// 			FileParser parser(file);

//         	parser.addCallback("Tile Name",  [&](const std::string&) { newTile->setName(parser.getNextLine()); } );
//         	parser.addCallback("Texture",    [&](const std::string&) { newTile->setMesh(std::stoi(parser.getNextLine())); } );
//         	parser.addCallback("Type",       [&](const std::string&) { newTile->setType(parser.getNextLine()); } );

// 			parser.run();

// 			tilesList.push_back(newTile);
// 			tilesDict[newTile->getName()] = nbTilesId;

// 			nbTilesId++; 
// 		}
// 	}

// 	TilesLoader::~TilesLoader()
// 	{
// 		LOG_THIS_MEMBER(DOM);

// 		tilesList.clear();
// 		tilesList.shrink_to_fit();
// 		//std::vector<TilesLoader::TilesId* >().swap(tilesList); //delete block list
// 	}

// 	const TilesLoader::TilesId* TilesLoader::getTile(int id) const
// 	{
// 		LOG_THIS_MEMBER(DOM);

// 		if(id < nbTilesId)
// 			return tilesList[id].get();
// 		else
// 			return nullptr;
// 	}

// 	const TilesLoader::TilesId* TilesLoader::getTile(std::string tileName) const
// 	{
// 		LOG_THIS_MEMBER(DOM);

// 		auto it = tilesDict.find(tileName);

// 		if(it != tilesDict.end())
// 			return tilesList[tilesDict.at(tileName)].get();
// 		else
// 			return nullptr;
// 	}
// }