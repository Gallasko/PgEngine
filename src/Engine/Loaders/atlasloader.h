#ifndef FONTLOADER_H
#define FONTLOADER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

#include "constant.h"

using namespace pg;

namespace pg
{
    // Type forwarding
    class OpenGLVertexArrayObject;
    class OpenGLBuffer;
}

class AtlasLoader
{
public:
    class AtlasElement
    {
    friend class AtlasLoader;
    public:
        AtlasElement();
        ~AtlasElement();

        inline int getId() const { return id; }
        inline std::string getName() const { return name; }
        inline OpenGLVertexArrayObject* getMesh() const { return VAO; }
        
        inline unsigned int getWidth() const { return width; }
        inline unsigned int getHeight() const { return height; }

        inline constant::SquareInfo getModelInfo() const { return modelInfo; }

    protected:
        inline void setId(int id) { this->id = id; }
        inline void setWidth(unsigned int w) { this->width = w; }
        inline void setHeight(unsigned int h) { this->height = h; }
        inline void setName(const std::string& name) { this->name = name; }
        void setMesh(unsigned int xPos, unsigned int yPos, unsigned int atlasWidth, unsigned int atlasHeight); 

    private:
        int id = 0;
        std::string name = "";
        unsigned int width = 0;
        unsigned int height = 0;

        constant::SquareInfo modelInfo;

        OpenGLVertexArrayObject *VAO;
        OpenGLBuffer *VBO;
        OpenGLBuffer *EBO;
    };

    AtlasLoader(const std::string& fontFile);
    ~AtlasLoader();

    const AtlasLoader::AtlasElement* getAtlasElement(int id) const;
    const AtlasLoader::AtlasElement* getAtlasElement(const std::string& charaName) const;

    inline unsigned int getAtlasWidth() const { return atlasWidth; }
    inline unsigned int getAtlasHeight() const { return atlasHeight; }

    bool isEmpty() const { return nbCharaId == 0; }

private:
    std::vector<std::shared_ptr<AtlasLoader::AtlasElement>> charaList;
    std::unordered_map<std::string, int> charaDict;
    int nbCharaId;

    unsigned int atlasWidth = 0;
    unsigned int atlasHeight = 0;
};

#endif