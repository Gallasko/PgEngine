#include "fontloader.h"

FontLoader::Font::Font()
{
	initializeOpenGLFunctions(); 

	VAO = new QOpenGLVertexArrayObject();
	VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

	VAO->create();
	VBO->create();
	EBO->create();
}

FontLoader::Font::~Font()
{
	delete VAO;
	delete VBO;
	delete EBO;
}

void FontLoader::Font::setMesh(unsigned int xPos, unsigned int yPos, unsigned int atlasWidth, unsigned int atlasHeight)
{
    float xMin = xPos / (float)atlasWidth;
    float xMax = (xPos + width) / (float)atlasWidth;
    
    float yMin = (yPos - height + 1) / (float)atlasHeight;
    float yMax = (yPos + 1) / (float)atlasHeight;

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

FontLoader::FontLoader(std::string fontFile) : nbCharaId(0)
{
	std::ifstream f;
	FontLoader::Font *newChara = nullptr;

	f.open(fontFile, std::ifstream::in);

    unsigned int xPos = 0;
    unsigned int yPos = 0;

	if(f.is_open())
	{
        std::cout << "Open Font file: " << fontFile << std::endl;

		for(std::string line; std::getline(f, line); )
		{
            if(line == "Atlas Width")
                if (std::getline(f, line))
                    atlasWidth = std::stoi(line);

            if(line == "Atlas Height")
                if (std::getline(f, line))
                    atlasHeight = std::stoi(line);

            if(line == "Opacity 1")
                if (std::getline(f, line))
                    opacity[0] = std::stoi(line);

            if(line == "Opacity 2")
                if (std::getline(f, line))
                    opacity[1] = std::stoi(line);

            if(line == "Opacity 3")
                if (std::getline(f, line))
                    opacity[2] = std::stoi(line);

            if(line == "Row")
            {
                if (std::getline(f, line))
                {
                    if(line == "Base Y")
                    {
                        if (std::getline(f, line))
                        {
                            xPos = 1;
                            yPos = std::stoi(line);
                        }
                    }
                }
            }
                
            if(line == "Charactere")
            {
                newChara = new FontLoader::Font();
                newChara->setId(nbCharaId);

                if (std::getline(f, line))
                    newChara->setName(line);
            }

            if(line == "Width")
                if (std::getline(f, line) && newChara != nullptr)
                    newChara->setWidth(std::stoi(line));
                
            if(line == "Height")
                if (std::getline(f, line) && newChara != nullptr)
                    newChara->setHeight(std::stoi(line));

            if(line == "Y-Offset")
                if (std::getline(f, line) && newChara != nullptr)
                    newChara->setOffset(std::stoi(line));

            if(line == "###########")
            {
                if (std::getline(f, line) && newChara != nullptr)
                {
                    newChara->setMesh(xPos, yPos, atlasWidth, atlasHeight);

                    charaList.push_back(newChara);
		            charaDict[newChara->getName()] = nbCharaId;

                    nbCharaId++;
                    xPos += newChara->getWidth() + 1;

                    std::cout << "Chara " << newChara->getName() << " Registered ";

                }
                    
            }
                
		}
    }

    f.close();
    std::cout << std::endl << "Font correctly loaded" << std::endl;
}

FontLoader::~FontLoader()
{
	std::vector<FontLoader::Font* >().swap(charaList); //delete block list
}

FontLoader::Font* FontLoader::getChara(int id) const
{
	if(id < nbCharaId)
		return charaList[id];
	else
		return charaList[0];
}

FontLoader::Font* FontLoader::getChara(std::string charaName) const
{
	auto it = charaDict.find(charaName);

	if(it != charaDict.end())
		return charaList[charaDict.at(charaName)];
	else
		return charaList[0];
}