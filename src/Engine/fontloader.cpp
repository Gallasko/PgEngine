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

	//texpos x                 texpos y
	modelInfo.vertices[3] =  xMin; modelInfo.vertices[4] =  yMin;   
	modelInfo.vertices[8] =  xMax; modelInfo.vertices[9] =  yMin;
	modelInfo.vertices[13] = xMin; modelInfo.vertices[14] = yMax;
	modelInfo.vertices[18] = xMax; modelInfo.vertices[19] = yMax;

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
}

FontLoader::FontLoader(const std::string& fontFile) : nbCharaId(0)
{
	std::ifstream f;
	FontLoader::Font *newChara = nullptr;

	f.open(fontFile, std::ifstream::in);

    unsigned int xPos = 0;
    unsigned int yPos = 0;

	if(f.is_open())
	{

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
                }
                    
            }
                
		}
    }

    f.close();
}

FontLoader::~FontLoader()
{
    charaList.clear();
    charaList.shrink_to_fit();
	//std::vector<FontLoader::Font* >().swap(charaList); //delete block list
}

FontLoader::Font* FontLoader::getChara(int id) const
{
	if(id < nbCharaId)
		return charaList[id];
	else
		return charaList[0];
}

FontLoader::Font* FontLoader::getChara(const std::string& charaName) const
{
	auto it = charaDict.find(charaName);

	if(it != charaDict.end())
		return charaList[charaDict.at(charaName)];
	else
		return charaList[0];
}