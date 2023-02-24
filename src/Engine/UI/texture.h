#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include "uisystem.h"
#include "constant.h"

#include "logger.h"

namespace pg
{
    class MasterRenderer;
    class Renderable;

    // Todo make a is relationship instead of inherit from UiComponent as it is like that with the new ECS
    // Todo make it a named component instead of an unamed one
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

    struct TextureComponentSystem : public System<Own<TextureComponent>, Ref<UiComponent>, StoragePolicy, InitSys>
    {
        TextureComponentSystem()
        {
            
        }

        virtual void init() override
        {
            auto group = registerGroup<UiComponent, TextureComponent>();

            group->addOnGroup([](Entity* entity) {
                LOG_INFO("Texture Component System", "Add entity " << entity->id << " to ui - tex group !");

                entity->world()->attach<Renderable>(entity);
            });
        }

    };

}