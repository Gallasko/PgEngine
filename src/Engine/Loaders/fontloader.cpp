#include "Helpers/openglobject.h"

#include "fontloader.h"

#include "Files/fileparser.h"
#include "logger.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "Font Loader";
    }

    FontLoader::Font::Font()
    {
        LOG_THIS_MEMBER(DOM);
    }

    FontLoader::Font::~Font()
    {
    }

    void FontLoader::Font::setMesh(unsigned int xPos, unsigned int yPos, unsigned int atlasWidth, unsigned int atlasHeight)
    {
        LOG_THIS_MEMBER(DOM);

        float xMin = xPos / (float)atlasWidth;
        float xMax = (xPos + width) / (float)atlasWidth;

        float yMin = (yPos - height + 1) / (float)atlasHeight;
        float yMax = (yPos + 1) / (float)atlasHeight;

        textureLimit.x = xMin;
        textureLimit.y = yMin;
        textureLimit.z = xMax;
        textureLimit.w = yMax;
    }

    FontLoader::FontLoader(const std::string& fontFile) : nbCharaId(0)
    {
        LOG_THIS_MEMBER(DOM);

        TextFile file = UniversalFileAccessor::openTextFile(fontFile);

        FileParser parser(file);
        std::shared_ptr<FontLoader::Font> newChara = nullptr;

        unsigned int xPos = 0;
        unsigned int yPos = 0;

        parser.addCallback("Atlas Width",  [&](const std::string&) { atlasWidth  = std::stoi(parser.getNextLine()); } );
        parser.addCallback("Atlas Height", [&](const std::string&) { atlasHeight = std::stoi(parser.getNextLine()); } );
        parser.addCallback("Opacity 1",    [&](const std::string&) { opacity[0] = std::stoi(parser.getNextLine()); } );
        parser.addCallback("Opacity 2",    [&](const std::string&) { opacity[1] = std::stoi(parser.getNextLine()); } );
        parser.addCallback("Opacity 3",    [&](const std::string&) { opacity[2] = std::stoi(parser.getNextLine()); } );
        parser.addCallback("Row",          [&](const std::string&) { if (parser.getNextLine() == "Base Y") { xPos = 1; yPos = std::stoi(parser.getNextLine()); } } );
        parser.addCallback("Charactere",   [&](const std::string&) { newChara = std::make_shared<FontLoader::Font>(); newChara->setId(nbCharaId); newChara->setName(parser.getNextLine()); } );
        parser.addCallback("Width",        [&](const std::string&) { if (newChara != nullptr) newChara->setWidth(std::stoi(parser.getNextLine())); } );
        parser.addCallback("Height",       [&](const std::string&) { if (newChara != nullptr) newChara->setHeight(std::stoi(parser.getNextLine())); } );
        parser.addCallback("Y-Offset",     [&](const std::string&) { if (newChara != nullptr) newChara->setOffset(std::stoi(parser.getNextLine())); } );
        
        parser.addCallback("###########",  [&](const std::string&) { if (newChara != nullptr) { 
            newChara->setMesh(xPos, yPos, atlasWidth, atlasHeight);
            charaList.push_back(newChara);
            charaDict[newChara->getName()] = nbCharaId;

            nbCharaId++;
            xPos += newChara->getWidth() + 1;
        } } );

        parser.run();
    }

    FontLoader::~FontLoader()
    {
        charaList.clear();
        charaList.shrink_to_fit();
    }

    const FontLoader::Font* FontLoader::getChara(int id) const
    {
        LOG_THIS_MEMBER(DOM);

        if (nbCharaId == 0)
            return nullptr;

        if (id < nbCharaId)
            return charaList[id].get();
        else
            return charaList[0].get();
    }

    const FontLoader::Font* FontLoader::getChara(const std::string& charaName) const
    {
        LOG_THIS_MEMBER(DOM);

        if (nbCharaId == 0)
        {
            LOG_ERROR(DOM, "Font is empty");
            return nullptr;
        }

        auto it = charaDict.find(charaName);

        if (it != charaDict.end())
            return charaList[charaDict.at(charaName)].get();
        else
        {
            LOG_ERROR(DOM, "Looking for " << charaName << ", but didn't find it in the dict");
            return charaList[0].get();
        }
    }
}