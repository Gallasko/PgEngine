#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include "uisystem.h"
#include "constant.h"

namespace pg
{
    struct TextureComponent : public UiComponent, private QOpenGLFunctions
    {
        TextureComponent(const UiSize& width, const UiSize& height, const std::string& textureName);
        TextureComponent(const UiComponent& component, const std::string& textureName);
        TextureComponent(const TextureComponent &rhs);
        virtual ~TextureComponent();

        inline void setTexture(const std::string& textureName) { this->textureName = textureName; }
        void generateMesh();

        virtual void render(MasterRenderer* masterRenderer);

        std::string textureName;

        constant::SquareInfo modelInfo;

        QOpenGLVertexArrayObject *VAO = nullptr;
        QOpenGLBuffer *VBO = nullptr;
        QOpenGLBuffer *EBO = nullptr;

        float oldWidth = width, oldHeight = height;

        bool initialised = false;
    };

}