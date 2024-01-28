#include "Helpers/openglobject.h"

#include "atlasloader.h"

#include "Files/fileparser.h"
#include "logger.h"

namespace
{
    static constexpr char const * DOM = "Atlas Element Loader";
}

AtlasLoader::AtlasElement::AtlasElement()
{
    LOG_THIS_MEMBER(DOM);

    VAO = new OpenGLVertexArrayObject();
    VBO = new OpenGLBuffer(OpenGLBuffer::VertexBuffer);
    EBO = new OpenGLBuffer(OpenGLBuffer::IndexBuffer);

    VAO->create();
    VBO->create();
    EBO->create();
}

AtlasLoader::AtlasElement::~AtlasElement()
{
    delete VAO;
    delete VBO;
    delete EBO;
}

void AtlasLoader::AtlasElement::setMesh(unsigned int xPos, unsigned int yPos, unsigned int atlasWidth, unsigned int atlasHeight)
{
    LOG_THIS_MEMBER(DOM);

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
    VBO->setUsagePattern(OpenGLBuffer::StreamDraw);
    VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // texture coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    EBO->bind();
    EBO->setUsagePattern(OpenGLBuffer::StreamDraw);
    EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

    VAO->release();
}

AtlasLoader::AtlasLoader(const std::string& texFile) : nbCharaId(0)
{
    LOG_THIS_MEMBER(DOM);

    TextFile file = UniversalFileAccessor::openTextFile(texFile);

    FileParser parser(file);
    std::shared_ptr<AtlasLoader::AtlasElement> newChara = nullptr;

    unsigned int xPos = 0;
    unsigned int yPos = 0;

    parser.addCallback("Atlas Width",  [&](const std::string&) { atlasWidth = std::stoi(parser.getNextLine()); } );
    parser.addCallback("Atlas Height", [&](const std::string&) { atlasHeight = std::stoi(parser.getNextLine()); } );
    parser.addCallback("Row",          [&](const std::string&) { if(parser.getNextLine() == "Base Y") { xPos = 0; yPos = std::stoi(parser.getNextLine()); } } );
    parser.addCallback("Element",      [&](const std::string&) { newChara = std::make_shared<AtlasLoader::AtlasElement>(); newChara->setId(nbCharaId); newChara->setName(parser.getNextLine()); } );
    parser.addCallback("Width",        [&](const std::string&) { if(newChara != nullptr) newChara->setWidth(std::stoi(parser.getNextLine())); } );
    parser.addCallback("Height",       [&](const std::string&) { if(newChara != nullptr) newChara->setHeight(std::stoi(parser.getNextLine())); } );

    parser.addCallback("###########",  [&](const std::string&) { if (newChara != nullptr) { 
        newChara->setMesh(xPos, yPos, atlasWidth, atlasHeight);
        charaList.push_back(newChara);
        charaDict[newChara->getName()] = nbCharaId;

        nbCharaId++;
        xPos += newChara->getWidth() + 1;
    } } );

    parser.run();
}

AtlasLoader::~AtlasLoader()
{
    charaList.clear();
    charaList.shrink_to_fit();
}

const AtlasLoader::AtlasElement* AtlasLoader::getAtlasElement(int id) const
{
    LOG_THIS_MEMBER(DOM);

    if (nbCharaId == 0)
        return nullptr;

    if(id < nbCharaId)
        return charaList[id].get();
    else
        return charaList[0].get();
}

const AtlasLoader::AtlasElement* AtlasLoader::getAtlasElement(const std::string& charaName) const
{
    LOG_THIS_MEMBER(DOM);

    if (nbCharaId == 0)
        return nullptr;

    auto it = charaDict.find(charaName);

    if(it != charaDict.end())
        return charaList[charaDict.at(charaName)].get();
    else
        return charaList[0].get();
}